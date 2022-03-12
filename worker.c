//
// Created by dran on 3/8/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>


#include "worker.h"


int process_job(job_t* current_job, struct resource_info* resource) {
    int ret_status;
    bool request_is_get;
    char request_url[MAX_URL_SIZE + 1];
    enum http_version request_version;
    bool request_keep_alive;
    char response_buffer[JOB_REQUEST_BUFFER_SIZE + 1];
    int response_tail;
    FILE* requested_file;

    // discard expired jobs
    if (current_job->expiration_time > (unsigned int) time(NULL)) {
        return TERMINATE;
    }

    // try to non-blocking read from TCP stream
    ret_status = recv(current_job->socket_fd, current_job->request + current_job->request_tail,
                      JOB_REQUEST_BUFFER_SIZE - current_job->request_tail, MSG_DONTWAIT);
    if (ret_status == -1) {  // no data available, push job back onto the stack
        return ENQUEUE;
    } else if (ret_status == 0) {  // connection closed
        return TERMINATE;
    } else {
        current_job->request_tail += ret_status;
        current_job->expiration_time = (unsigned int) time(NULL) + KEEP_ALIVE_TIMEOUT;
    }

    ret_status = parse_request_string(current_job->request, &request_is_get, request_url,
                                      &request_version, &request_keep_alive);
    if (ret_status == FAIL && current_job->request_tail == (JOB_REQUEST_BUFFER_SIZE - 1)) {
        request_version = MALFORMED;  // no CRLF found and buffer filled, Bad Request, Bad!
    } else if (ret_status == FAIL) {
        return ENQUEUE;  // no CRLF found, re-enqueue and wait for more data (possible Slow Loris)
    }

    // build response header first line
    if (request_version == MALFORMED) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.1 400 Bad Request\r\n");
        ret_status = try_send_in_chunks(current_job->socket_fd, response_buffer, response_tail);
        return TERMINATE;
    } else if (request_version == NOT_SUPPORTED) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.1 505 HTTP Version Not Supported\r\n");
        ret_status = try_send_in_chunks(current_job->socket_fd, response_buffer, response_tail);
        return TERMINATE;
    } else if (request_version == DOT_ZERO) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.0 ");
    } else if (request_version == DOT_ONE) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.1 ");
    }

    // try to open file for GET and or build response
    if (!request_is_get) {
        copy_into_buffer(response_buffer, &response_tail,
                         "405 Method Not Allowed\r\n");
        requested_file = NULL;
    } else {
        requested_file = fill_content_info(response_buffer,&response_tail, request_url);
    }

    // additional response headers
    if (request_keep_alive) {
        copy_into_buffer(response_buffer, &response_tail,
                         "Connection: keep-alive\r\n");
    } else {
        copy_into_buffer(response_buffer, &response_tail,
                         "Connection: close\r\n");
    }

    // send the response we've built over
    ret_status = try_send_in_chunks(current_job->socket_fd, response_buffer, response_tail);
    if (ret_status == FAIL) {
        safe_write(resource->std_out,"Failed to send to client\n");
        return TERMINATE;
    }

    if (requested_file != NULL) {
        // clear buffer and start sending over the file
        memset(response_buffer, '\0', JOB_REQUEST_BUFFER_SIZE);
        response_tail = 0;
        copy_into_buffer(response_buffer, &response_tail,
                         "\r\n");
        ret_status = fread(response_buffer + response_tail, sizeof(char), JOB_REQUEST_BUFFER_SIZE - response_tail, requested_file);
        if (ret_status != 0) {
            response_tail += ret_status;
            ret_status = try_send_in_chunks(current_job->socket_fd, response_buffer, response_tail);
            if (ret_status == FAIL) {
                safe_write(resource->std_out,"Failed to send to client\n");
                return TERMINATE;
            }
        }
    }

    if (request_keep_alive) {
        current_job->expiration_time = (unsigned int) time(NULL) + KEEP_ALIVE_TIMEOUT;
        return ENQUEUE;
    } else {
        return TERMINATE;
    }
}

int parse_request_string(char* working_request, bool* is_get, char* url, enum http_version* version, bool* keep_alive) {
    char* end_of_first_line;
    char* divider_a;
    char* divider_b;
    char* end_of_url;

    // will not parse request line unless there is a complete line
    end_of_first_line = strchr(working_request, '\n');
    if (end_of_first_line == NULL) {
        return FAIL;
    }

    // parse request line
    // starting with http version
    *end_of_first_line = '\0';
    divider_b = strrchr(working_request, ' ');  // TODO : what if the parts of a request line is seperated by more than one space char?
    if (divider_b == NULL) {
        *version = MALFORMED;
        return SUCCESS;
    }

    divider_b ++;
    if (!matches_command(divider_b, "HTTP/")) {
        *version = MALFORMED;
        return SUCCESS;
    } else if (matches_command(divider_b, "HTTP/1.1")){
        *version = DOT_ONE;
    } else if (matches_command(divider_b, "HTTP/1.0")) {
        *version = DOT_ZERO;
    } else {
        *version = NOT_SUPPORTED;
        return SUCCESS;
    }

    // request method
    if (matches_command(working_request, "GET")) {
        *is_get = true;
    } else if (matches_command(working_request, "HEAD")) {
        *is_get = false;
    } else if (matches_command(working_request, "POST")) {
        *is_get = false;
    } else if (matches_command(working_request, "PUT")) {
        *is_get = false;
    } else if (matches_command(working_request, "DELETE")) {
        *is_get = false;
    } else if (matches_command(working_request, "CONNECT")) {
        *is_get = false;
    } else if (matches_command(working_request, "TRACE")) {
        *is_get = false;
    } else if (matches_command(working_request, "PATCH")) {
        *is_get = false;
    } else if (matches_command(working_request, "OPTIONS")) {
        *is_get = false;
    }else {
        *version = MALFORMED;
        return SUCCESS;
    }

    // copy url
    divider_a = strchr(working_request, ' ');
    divider_a ++;
    divider_b --;
    *divider_b = '\0';
    if (divider_a >= divider_b) {
        *version = MALFORMED;
        return SUCCESS;
    } else {
        strncpy(url, divider_a, MAX_URL_SIZE);
        // strip the last char if it is '/', except when url is "/"
        end_of_url = strrchr(url, '/');
        if (end_of_url != url && (end_of_url + 1) == strrchr(url, '\0')) {
            *end_of_url = '\0';
        }
    }

    *end_of_first_line = '\n';
    // parse any request headers
    *keep_alive = false;
    divider_a = end_of_first_line + 1;
    divider_b = strchr(divider_a, '\n');
    while (divider_b != NULL) {
        if (matches_command_case_insensitive(divider_a, "Connection: keep-alive")) {
            *keep_alive = true;
        }
        divider_a = divider_b + 1;
        divider_b = strchr(divider_a, '\n');
    }
    return SUCCESS;
}

int matches_command(char* target, char* command) {
    if (strncmp(target, command, strlen(command)) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int matches_command_case_insensitive(char* target, char* command) {
    if (strncasecmp(target, command, strlen(command)) == 0) {
        return 1;
    } else {
        return 0;
    }
}

FILE* fill_content_info(char* response_buffer, int* response_tail, char* url) {
    int ret_status;
    char file_path[JOB_REQUEST_BUFFER_SIZE + 1];
    FILE* requested_file;
    struct stat file_stats;
    char* file_name_start;
    char* file_extension_start;

    file_name_start = strrchr(url, '/');
    file_extension_start = strrchr(file_name_start, '.');
    if (file_extension_start == NULL) {  // url is a directory, try to open url/index.html or url/index.htm
        sprintf(file_path, "%s%s/index.html", DOCUMENT_ROOT, url);
        ret_status = stat(file_path, &file_stats);
        if ((ret_status == -1) && (errno == ENOENT)) {
            sprintf(file_path, "%s%s/index.htm", DOCUMENT_ROOT, url);
            ret_status = stat(file_path, &file_stats);
            if ((ret_status == -1) && (errno == ENOENT)) {
                copy_into_buffer(response_buffer, response_tail,
                         "404 Not Found\r\n");
                return NULL;
            } else if ((ret_status == -1) && (errno == EACCES)) {
                copy_into_buffer(response_buffer, response_tail,
                                 "405 Forbidden\r\n");
                return NULL;
            }
        } else if ((ret_status == -1) && (errno == EACCES)) {
            copy_into_buffer(response_buffer, response_tail,
                             "405 Forbidden\r\n");
            return NULL;
        }
    } else {  // try to get file stats on requested file
        sprintf(file_path, "%s%s", DOCUMENT_ROOT, url);
        ret_status = stat(file_path, &file_stats);
        if ((ret_status == -1) && (errno == ENOENT)) {
            copy_into_buffer(response_buffer, response_tail,
                             "404 Not Found\r\n");
            return NULL;
        } else if ((ret_status == -1) && (errno == EACCES)) {
            copy_into_buffer(response_buffer, response_tail,
                             "405 Forbidden\r\n");
            return NULL;
        }
    }

    // try to open file
    requested_file = fopen(file_path, "rb");
    if (requested_file == NULL) {
        // 405 because we know file stat exist
        copy_into_buffer(response_buffer, response_tail,
                         "405 Forbidden\r\n");
        return NULL;
    } else {
        copy_into_buffer(response_buffer, response_tail,
                         "200 OK\r\n");
    }

    if (matches_command(file_extension_start, ".html")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: text/html\r\n");
    } else if (matches_command(file_extension_start, ".txt")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: text/plain\r\n");
    } else if (matches_command(file_extension_start, ".png")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: image/png\r\n");
    } else if (matches_command(file_extension_start, ".gif")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: image/gif\r\n");
    } else if (matches_command(file_extension_start, ".jpg")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: image/jpg\r\n");
    } else if (matches_command(file_extension_start, ".css")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: text/css\r\n");
    } else if (matches_command(file_extension_start, ".js")) {
        copy_into_buffer(response_buffer, response_tail, "Content-Type: application/javascript\r\n");
    } else {
        // 405 or whatever better convey file type not supported
        copy_into_buffer(response_buffer, response_tail,
                         "405 Forbidden\r\n");
        return NULL;
    }

    // add content length header
    // borrowing file path buffer
    sprintf(file_path, "Content-Length: %ld\r\n", file_stats.st_size);
    copy_into_buffer(response_buffer, response_tail, file_path);
    return requested_file;
}

void copy_into_buffer(char* target, int* target_tail, char* content) {
    strcpy(target + *target_tail, content);  // TODO: address possible buffer overflow here
    *target_tail += strlen(content);
}

int try_send_in_chunks(int socket_fd, char* buffer, int length) {
    int ret_status;
    int bytes_sent;
    char garbage_buffer[10];

    bytes_sent = 0;
    while (bytes_sent < length) {

        // first see if client is still there
        ret_status = recv(socket_fd, garbage_buffer, 9, MSG_PEEK | MSG_DONTWAIT);
        if (ret_status == 0) {
            return FAIL;
        }
        // do send based on what's already sent;
        ret_status = send(socket_fd, buffer + bytes_sent, length - bytes_sent, 0);
        if (ret_status == -1) {
            // not sure what will cause this,
            // it's my understanding that if client is not there the whole process just get killed
            return FAIL;
        }
        bytes_sent += ret_status;
    }
    return SUCCESS;
}