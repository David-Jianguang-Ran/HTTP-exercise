//
// Created by dran on 3/8/22.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "worker.h"

#define DEBUG 1
#define DEBUG_THREADS 0

int CACHE_TTL;

struct resource_info create_shared_resource(int job_stack_size, int reserve_slots) {
    struct resource_info created;
    created.thread_id = -1;
    created.client_job_stack = job_stack_construct(job_stack_size, reserve_slots);
    // created.cache_job_stack = job_stack_construct(job_stack_size, reserve_slots);
    created.std_out = safe_init(stdout);
    created.addr_lookup_lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(created.addr_lookup_lock, NULL);
    created.block_table = block_table_create();
    return created;
}

void free_shared_resource(struct resource_info* ptr_to_resource) {
    job_stack_destruct(ptr_to_resource->client_job_stack);
    safe_close(ptr_to_resource->std_out);
    pthread_mutex_destroy(ptr_to_resource->addr_lookup_lock);
}

//void* worker_main(void* shared) {
//    char output_buffer[PRINTOUT_BUFFER_SIZE];
//    struct resource_info* shared_resource;
//    int ret_status;
//    job_t* current_job;
//    shared_resource = (struct resource_info*) shared;
//    while (1) {
//        ret_status = job_stack_pop(shared_resource->job_stack, &current_job);
//        if (ret_status == FINISHED) {
//            return NULL;
//        } else if (ret_status == SUCCESS) {
//            if (DEBUG_THREADS) {
//                sprintf(output_buffer, "<%d> socket:%d SERVICE START\n", shared_resource->thread_id, current_job->socket_fd);
//                safe_write(shared_resource->std_out, output_buffer);
//            }
//            ret_status = process_job(current_job, shared_resource);  // this is where most of the work is done
//            if (ret_status == TERMINATE) {
//                if (DEBUG_THREADS) {
//                    sprintf(output_buffer, "<%d> socket:%d FINISHED\n", shared_resource->thread_id, current_job->socket_fd);
//                    safe_write(shared_resource->std_out, output_buffer);
//                }
//                job_destruct(current_job);
//            } else {  // re-enqueue job, expire time already set during `process_job`
//                ret_status = job_stack_push_back(shared_resource->job_stack, current_job);
//                if (ret_status != SUCCESS) {
//                    if (DEBUG_THREADS) {
//                        sprintf(output_buffer, "<%d> socket:%d FINISHED\n", shared_resource->thread_id, current_job->socket_fd);
//                        safe_write(shared_resource->std_out, output_buffer);
//                    }
//                    job_destruct(current_job);
//                } else {
//                    if (DEBUG_THREADS) {
//                        sprintf(output_buffer, "<%d> socket:%d RE-ENQUEUE\n", shared_resource->thread_id, current_job->socket_fd);
//                        safe_write(shared_resource->std_out, output_buffer);
//                    }
//                }
//            }
//        }
//    }
//
//}

int process_job(job_t* current_job, struct resource_info* shared_resource) {
    int ret_status;
    bool request_is_get;
    char path[MAX_URL_SIZE + 1];
    char hostname[MAX_NAME_LENGTH + 1];
    enum http_version request_version;
    bool request_keep_alive;

    struct addrinfo* server_address;
    enum host_status server_status;

    char output_buffer[PRINTOUT_BUFFER_SIZE + JOB_REQUEST_BUFFER_SIZE];
    char response_buffer[JOB_REQUEST_BUFFER_SIZE + 1];
    int response_tail;


    // discard expired jobs
    if ((current_job->expiration_time != 0) && (current_job->expiration_time < (unsigned int) time(NULL))) {
        if (DEBUG) {
            sprintf(output_buffer, "<%d> socket:%d Timed Out\n", shared_resource->thread_id, current_job->client_socket_fd);
            safe_write(shared_resource->std_out, output_buffer);
        }
        return TERMINATE;
    }

    // try to non-blocking read from TCP stream
    ret_status = recv(current_job->client_socket_fd, current_job->request + current_job->request_tail,
                      JOB_REQUEST_BUFFER_SIZE - current_job->request_tail, MSG_DONTWAIT);
    if (ret_status == -1) {  // no data available, push job back onto the stack
        return ENQUEUE;
    } else if (ret_status == 0) {  // connection closed
        return TERMINATE;
    } else {
        current_job->request_tail += ret_status;
        current_job->expiration_time = (unsigned int) time(NULL) + KEEP_ALIVE_TIMEOUT;
    }

    ret_status = parse_request_string(current_job->request, &request_is_get, hostname, path,
                                      &request_version, &request_keep_alive);

    if (ret_status == FAIL && current_job->request_tail == (JOB_REQUEST_BUFFER_SIZE - 1)) {
        request_version = MALFORMED;  // no CRLF found and buffer filled, Bad Request, Bad!
    } else if (ret_status == FAIL) {
        return ENQUEUE;  // no CRLF found, re-enqueue and wait for more data (possible Slow Loris)
    }

    if (DEBUG && (request_version == MALFORMED || request_version == NOT_SUPPORTED || !request_is_get)) {
        sprintf(output_buffer, "<%d> socket:%d request-line:\n%s\n4xx response\n", shared_resource->thread_id, current_job->client_socket_fd, current_job->request);
        safe_write(shared_resource->std_out, output_buffer);
    }

    // build response header first line
    if (request_version == MALFORMED) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.1 400 Bad Request\r\n");
        try_send_in_chunks(current_job->client_socket_fd, response_buffer, response_tail);
        return TERMINATE;
    } else if (request_version == NOT_SUPPORTED) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.1 505 HTTP Version Not Supported\r\n");
        try_send_in_chunks(current_job->client_socket_fd, response_buffer, response_tail);
        return TERMINATE;
    } else if (request_version == DOT_ZERO) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.0 ");
    } else if (request_version == DOT_ONE) {
        copy_into_buffer(response_buffer, &response_tail,
                         "HTTP/1.1 ");
    }

    if (1) {  // always printout first line of a valid request for inspection
        sprintf(output_buffer, "<%d> socket:%d request-line:\n%s\nKeep-Alive:%d\n", shared_resource->thread_id, current_job->client_socket_fd, current_job->request, request_keep_alive);
        safe_write(shared_resource->std_out, output_buffer);
    }

    // resolve hostname, check hostname against block table
    server_status = resolve_host(shared_resource, hostname, &server_address);
    if (!request_is_get) {
        copy_into_buffer(response_buffer, &response_tail,
                         "405 Method Not Allowed\r\n");
    } else if (server_status == blocked) {
        copy_into_buffer(response_buffer, &response_tail,
                         "403 Forbidden\r\n");
    } else if (server_status == not_found) {
        copy_into_buffer(response_buffer, &response_tail,
                         "404 Not Found\r\n");
    } else if (server_status == ok) {
        // respond to client either from cache or web server
        ret_status = handle_valid_request(current_job, shared_resource, server_address,
                                        path, hostname, response_buffer, &response_tail);
        return TERMINATE;
    }

    // additional response headers
    if (request_keep_alive) {
        copy_into_buffer(response_buffer, &response_tail,
                         "Proxy-Connection: close\r\n\r\n");
    }

    // send the response we've built over the wire
    ret_status = try_send_in_chunks(current_job->client_socket_fd, response_buffer, response_tail);
    if (ret_status == FAIL) {
        safe_write(shared_resource->std_out,"Failed to send to client\n");
    }

    // no more keep alive with proxies
    return TERMINATE;
}


enum host_status resolve_host(struct resource_info* shared_resource, char* hostname, struct addrinfo** server_address) {
    int ret_status;
    struct addrinfo server_address_hints;
    struct addrinfo* server_address_head;
    char server_name[MAX_NAME_LENGTH + 1];  // local copy of host name because we don't want to mess with outer scope data
    char* server_port;

    *server_address = NULL;
    // break hostname into name and port
    strcpy(server_name, hostname);
    server_port = strchr(server_name, ':');
    if (server_port != NULL) {
        *server_port = '\0';
        server_port++;
    }

    if (block_table_check(shared_resource->block_table, server_name)) {
        return blocked;
    }

    // resolve address, this is protected by addr_lookup_lock
    pthread_mutex_lock(shared_resource->addr_lookup_lock);
    server_address_hints.ai_family = AF_UNSPEC;
    server_address_hints.ai_socktype = SOCK_STREAM;
    if (server_port != NULL) {
        ret_status = getaddrinfo(server_name, server_port, &server_address_hints, server_address);
    } else {
        ret_status = getaddrinfo(server_name, "http", &server_address_hints, server_address);
    }
    pthread_mutex_unlock(shared_resource->addr_lookup_lock);
    if (ret_status != 0) {
        return not_found;
    }

    // un-mangle hostname
    if (server_port != NULL) {
        server_port--;
        *server_port = ':';
    }

    // check the resolved address(es) against block table
    server_address_head = *server_address;
    while (1) {
        inet_ntop(AF_INET, &(((struct sockaddr_in*)(*server_address)->ai_addr)->sin_addr), server_name, MAX_NAME_LENGTH);
        // printf("checking addr: %s\n", server_name);
        if (block_table_check(shared_resource->block_table, server_name)) {
            if ((*server_address)->ai_next != NULL) {
                (*server_address) = (*server_address)->ai_next;
            } else {
                freeaddrinfo(server_address_head);
                *server_address = NULL;
                return blocked;
            }
        } else {

            return ok;
        }
    }
}

int handle_valid_request(job_t* current_job, struct resource_info* shared_resource, struct addrinfo* server_address,
                         char* path, char* hostname, char* response_buffer, int* response_tail) {
    int result;
    char* url_question_mark;
    int server_socket_fd;
    cache_record_t* cache_record;
    enum action_status cache_action;
    char cache_file_path[MAX_NAME_LENGTH + 1];
    FILE* cache_file;
    int bytes_read;
    int response_header_length;
    int response_content_length;
    int response_content_read;

    // clear response buffer
    memset(response_buffer, 0, JOB_REQUEST_BUFFER_SIZE + 1);
    *response_tail = 0;
    cache_record = NULL;

    // check request against cache
    url_question_mark = strchr(path, '?');
    if (url_question_mark != NULL) {
        // borrowing response buffer to rebuild url
        sprintf(response_buffer, "%s%s", hostname, path);
        cache_record = cache_record_get_or_create(shared_resource->cache_table, response_buffer, &cache_action);
        memset(response_buffer, 0, JOB_REQUEST_BUFFER_SIZE + 1);
    } else {
        cache_action = unavailable;
    }

    // TODO : prefetch :  a job without client socket mean it is a prefetch cache job cache hit, do nothing
    if (cache_action == should_read) {
        // open cache file and respond
        sprintf(cache_file_path, "%s/%s", CACHE_FILE_ROOT, cache_record->name);
        cache_file = fopen(cache_file_path, "r");
        if (cache_file != NULL) {
            bytes_read = fread(response_buffer, sizeof(char), JOB_REQUEST_BUFFER_SIZE, cache_file);
            response_buffer[bytes_read] = '\0';
            while (bytes_read != 0) {
                *response_tail = bytes_read;
                result = try_send_in_chunks(current_job->client_socket_fd, response_buffer, *response_tail);
                if (result == FAIL) {
                    // log failure, although it is unlikely to happen
                    return FAIL;
                }
                bytes_read = fread(response_buffer, sizeof(char), JOB_REQUEST_BUFFER_SIZE, cache_file);
                response_buffer[bytes_read] = '\0';
            }
            // TODO : try to start prefetch here
            fclose(cache_file);
            cache_record_close(cache_record, cache_action);
            return SUCCESS;
        }
    }

    if (cache_action == should_write) {
        // open file for cache write
        sprintf(cache_file_path, "%s/%s", CACHE_FILE_ROOT, cache_record->name);
        cache_file = fopen(cache_file_path, "w");
    }

    // open a socket to server
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    result = connect(server_socket_fd, server_address->ai_addr, server_address->ai_addrlen);
    if (result == -1) {
        if (cache_file != NULL) {
            fclose(cache_file);
        }
        return FAIL;
    }

    // built a new request then send to server
    sprintf(response_buffer, "GET %s HTTP/1.1\r\nHost: %s\r\n", path, hostname);
    result = try_send_in_chunks(server_socket_fd, current_job->request, current_job->request_tail);
    if (result == FAIL) {
        if (cache_file != NULL) {
            fclose(cache_file);
        }
        return FAIL;
    }

    // relay response, cache response if possible
    bytes_read = recv(server_socket_fd, response_buffer, JOB_REQUEST_BUFFER_SIZE, 0);
    parse_response_header(response_buffer, &response_header_length, &response_content_length);
    // store chunk in cache and send to client
    result = cache_and_send(current_job->client_socket_fd, cache_file, response_buffer, bytes_read);
    if (result == FAIL) {
        return FAIL;
    }
    // response may arrive in chunks with max size equal to response buffer max
    // keep receiving, caching and sending untill done
    if (bytes_read < response_header_length + response_content_length) {
        response_content_read = bytes_read - response_header_length;
        while (response_content_length > response_content_read) {
            bytes_read = recv(server_socket_fd, response_buffer, JOB_REQUEST_BUFFER_SIZE, 0);
            response_content_read += bytes_read;
            result = cache_and_send(current_job->client_socket_fd, cache_file, response_buffer, bytes_read);
            if (result == FAIL) {
                return FAIL;
            }
        }
    }
    // TODO : try to start prefetch here
    if (cache_action == should_write) {
        fclose(cache_file);
        cache_record_close(cache_record, should_write);
    }
    return SUCCESS;
}

int cache_and_send(int client_socket, FILE* cache_file,char* buffer, int length) {
    if (cache_file != NULL) {
        fwrite(buffer, sizeof(char), length, cache_file);
    }

    if (client_socket != -1) {
        return try_send_in_chunks(client_socket, buffer, length);
    }

    return SUCCESS;
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