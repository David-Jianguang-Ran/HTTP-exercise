//
// Created by dran on 4/9/22.
//

#ifndef NS_PA_2_PARSING_H
#define NS_PA_2_PARSING_H


#include <stdbool.h>
#include <stdio.h>

#include "constants.h"

enum http_version {MALFORMED, NOT_SUPPORTED, DOT_ZERO, DOT_ONE};

// request string must be null terminated or face UNDEFINED CONSEQUENCES!!
// will remove trailing / from url, unless it's only a slash
int parse_request_string(char* request_string, bool* is_GET, char* hostname, char* path,
                         enum http_version* version, bool* proxy_keep_alive);
// find the length of response header in bytes, including double /r/n
// extracts content-length from header
void parse_response_header(char* response_string, int* response_header_length, int* response_content_length);

// can only find 1 href after the current location of file
// the current location of file will be set to the closing " of the found href
// will return FINISHED upon reaching the end of text file
// SUCCESS for finding a link, FAIL for finding none
// links must have the following format href="link_content"
int find_href(FILE* text_file, char* link_buffer, int link_buffer_length);

// simple wrapper for strncmp, only match up to strlen(command)
// all strings must be zero terminated
// return 0 for no match, 1 for match
int matches_command(char* target, char* command);
int matches_command_case_insensitive(char* target, char* command);


#endif //NS_PA_2_PARSING_H
