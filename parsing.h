//
// Created by dran on 4/9/22.
//

#ifndef NS_PA_2_PARSING_H
#define NS_PA_2_PARSING_H


#include <stdbool.h>

#include "constants.h"

enum http_version {MALFORMED, NOT_SUPPORTED, DOT_ZERO, DOT_ONE};

// request string must be null terminated or face UNDEFINED CONSEQUENCES!!
// will remove trailing / from url, unless it's only a slash
int parse_request_string(char* request_string, bool* is_GET, char* path, char* hostname,
                         enum http_version* version, bool* proxy_keep_alive);
// find the length of response header in bytes, including double /r/n
// extracts content-length from header
void parse_response_header(char* response_string, int* response_header_length, int* response_content_length);


// simple wrapper for strncmp, only match up to strlen(command)
// all strings must be zero terminated
// return 0 for no match, 1 for match
int matches_command(char* target, char* command);
int matches_command_case_insensitive(char* target, char* command);


#endif //NS_PA_2_PARSING_H
