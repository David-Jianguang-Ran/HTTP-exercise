//
// Created by dran on 4/9/22.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "parsing.h"


int parse_request_string(char* working_request, bool* is_get, char* hostname, char* path,
                         enum http_version* version, bool* proxy_keep_alive) {
    char* end_of_first_line;
    char* divider_a;
    char* divider_b;
    char* slash;

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
    } else if (!matches_command_case_insensitive(divider_a, "http://")) {
        *version = MALFORMED;
    } else {
        slash = strchr(divider_a + 7, '/');
        strncpy(hostname, divider_a + 7, slash - divider_a - 7);
        strncpy(path, slash, MAX_URL_SIZE);
        // strip the last char if it is '/', except when url is "/"
        slash = strrchr(path, '/');
        if (slash != path && (slash + 1) == strrchr(path, '\0')) {
            *slash = '\0';
        }
    }
    *divider_b = ' ';
    // *end_of_first_line = '\n';

    // parse any request headers
    *proxy_keep_alive = false;
    divider_a = end_of_first_line + 1;
    divider_b = strchr(divider_a, '\n');
    while (divider_b != NULL) {
        if (matches_command_case_insensitive(divider_a, "Proxy-Connection: keep-alive")) {
            *proxy_keep_alive = true;
        } else if (matches_command_case_insensitive(divider_a, "Host: ")) {
            // capture hostname
            divider_a = strchr(divider_a, ' ');
            while (*divider_a == ' ') {
                divider_a++;
            }
            strncpy(hostname, divider_a, divider_b - divider_a - 2);
        }
        divider_a = divider_b + 1;
        divider_b = strchr(divider_a, '\n');
    }
    return SUCCESS;
}

void parse_response_header(char* response_string, int* response_header_length, int* response_content_length) {
    char* divider_a;
    char* divider_b;

    *response_header_length = -1;
    *response_content_length = -1;
    // parse any response headers
    divider_a = response_string;
    divider_b = strchr(divider_a, '\n');
    while (divider_b != NULL) {
        if (matches_command_case_insensitive(divider_a, "Content-length: ")) {
            divider_a = strchr(divider_a, ' ');
            *response_content_length = atoi(divider_a);
            break;
        }
        divider_a = divider_b + 1;
        divider_b = strchr(divider_a, '\n');
    }
    if (*response_content_length == -1) {  // header only, no body
        divider_b = strrchr(response_string, '\n');
        *response_header_length = divider_b - response_string + 1;
    } else {
        // find double /r/n
        divider_a = strchr(response_string, '\r');
        while (divider_a != NULL) {
            if (matches_command(divider_a, "\r\n\r\n")) {
                *response_header_length = divider_a - response_string + 4;
                return;
            } else {
                divider_a = strchr(divider_a + 1, '\r');
            }
        }
    }
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