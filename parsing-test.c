//
// this is a little jank, but whatever, it's school not real life
// Created by dran on 3/8/22.
//

#include <stdio.h>
#include <string.h>

#include "parsing.h"


int test_parse_request_string() {
    char request_string[JOB_REQUEST_BUFFER_SIZE + 1];
    bool is_get;
    char url[MAX_URL_SIZE];
    char hostname[MAX_URL_SIZE];
    enum http_version version;
    bool keep_alive;
    int result;

    strncpy(request_string, "GET http://none.exist.com/index.html HTTP/1.1\r\n"
                            "Host: none.exist.com\r\n"
                            "User-Agent: Wget/1.20.3 (linux-gnu)\r\n"
                            "Accept: */*\r\n"
                            "Accept-Language: en-US,en;q=0.5\r\n"
                            "Accept-Encoding: identity\r\n"
                            "Connection: keep-alive\r\n"
                            "Proxy-Connection: Keep-Alive\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, hostname, url,  &version, &keep_alive);
    printf("---------case 1---------\n"
           "%s\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "Host: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
           ,request_string, SUCCESS, result,
           true, is_get,
           "http://none.exist.com/index.html", url,
           "none.exist.com", hostname,
           DOT_ONE, version,
           true, keep_alive);

    strncpy(request_string, "GET https://none.exist.com/index.html HTTP/1.1\r\n"
                            "Host: none.exist.com\r\n"
                            "User-Agent: Wget/1.20.3 (linux-gnu)\r\n"
                            "Accept: */*\r\n"
                            "Accept-Language: en-US,en;q=0.5\r\n"
                            "Accept-Encoding: identity\r\n"
                            "Connection: keep-alive\r\n"
                            "Proxy-Connection: Keep-Alive\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, hostname, url,  &version, &keep_alive);
    printf("---------case 2---------\n"
           "%s\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "Host: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            ,request_string, SUCCESS, result,
           -1, is_get,
           "?", url,
           "?", hostname,
           NOT_SUPPORTED, version,
           -1, keep_alive);

    strncpy(request_string, "GET http://prefetch.job.com/file.jpg HTTP/1.1\r\n"
                            "Host: prefetch.job.com\r\n"
                            "Accept: */*\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, hostname, url,  &version, &keep_alive);
    printf("---------case 3---------\n"
           "%s\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "Host: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            ,request_string, SUCCESS, result,
           true, is_get,
           "http://prefetch.job.com/file.jpg", url,
           "prefetch.job.com", hostname,
           DOT_ONE, version,
           false, keep_alive);

    strncpy(request_string, "GET http://lazy.request.no.host/file.jpg HTTP/1.1\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, hostname, url,  &version, &keep_alive);
    printf("---------case 4---------\n"
           "%s\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "Host: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            ,request_string, SUCCESS, result,
           true, is_get,
           "/file.jpg", url,
           "lazy.request.no.host", hostname,
           DOT_ONE, version,
           false, keep_alive);

    return 0;
}

int main(int argc, char* argv[]) {
    return test_parse_request_string();
}