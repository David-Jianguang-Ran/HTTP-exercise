//
// this is a little jank, but whatever, it's school not real life
// Created by dran on 3/8/22.
//


#include "worker.c"


int test_parse_request_string() {
    char request_string[JOB_REQUEST_BUFFER_SIZE + 1];
    bool is_get;
    char url[MAX_URL_SIZE];
    enum http_version version;
    bool keep_alive;
    int result;

    strncpy(request_string, "GET /path/to/resource HTTP/1.1\r\n"
                            "Host: localhost:8000\r\n"
                            "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:97.0) Gecko/20100101 Firefox/97.0\r\n"
                            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
                            "Accept-Language: en-US,en;q=0.5\r\n"
                            "Accept-Encoding: gzip, deflate\r\n"
                            "Connection: keep-alive\r\n"
                            "Upgrade-Insecure-Requests: 1\r\n"
                            "Sec-Fetch-Dest: document\r\n"
                            "Sec-Fetch-Mode: navigate\r\n"
                            "Sec-Fetch-Site: none\r\n"
                            "Sec-Fetch-User: ?1\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 1---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
           , SUCCESS, result,
           true, is_get,
           "/path/to/resource", url,
           DOT_ONE, version,
           true, keep_alive);

    strncpy(request_string, "GET /Short/path HTTP/1.0\r\n"
                            "Host: localhost:8000\r\n"
                            "CONnection: keep-aLIvE\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 2---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           true, is_get,
           "/Short/path", url,
           DOT_ZERO, version,
           true, keep_alive);

    strncpy(request_string, "GET HTTP/1.0\r\n"
                            "Host: localhost:8000\r\n"
                            "CONnection: keep-aLIvE\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 3---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           true, is_get,
           "???", url,
           MALFORMED, version,
           -1, keep_alive);

    strncpy(request_string, "Not a request at all\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 4---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           -1, is_get,
           "???", url,
           MALFORMED, version,
           -1, keep_alive);

    strncpy(request_string, "GET-a-load-of-this-long-garbage-string-that-looks-like-HTTP/1.1\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 5---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           -1, is_get,
           "???", url,
           MALFORMED, version,
           -1, keep_alive);

    strncpy(request_string, "GET / HTTP/1.1\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 6---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           true, is_get,
           "/", url,
           DOT_ONE, version,
           false, keep_alive);

    strncpy(request_string,
            "PUT /vulnerable/data/make/sure/no/one/overwrites/it HTTP/1.1\r\n"
                "Connection: close\r\n"
            , JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 7---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           false, is_get,
           "/vulnerable/data/make/sure/no/one/overwrites/it", url,
           DOT_ONE, version,
           false, keep_alive);

    strncpy(request_string,
            "OPTIONS /will/you/not/hang/up/even/if/im/clueless HTTP/1.1\r\n"
            "Connection: keep-alive\r\n"
            , JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 8---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           false, is_get,
           "/will/you/not/hang/up/even/if/im/clueless", url,
           DOT_ONE, version,
           true, keep_alive);

    strncpy(request_string, "GET /valid/request/if/you/like/HTTP/0.9\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 9---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           -1, is_get,
           "/valid/request/if/you/like/HTTP/0.9", url,
           MALFORMED, version,
           -1, keep_alive);

    strncpy(request_string, "GET HTTP/1.1\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 10---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , SUCCESS, result,
           -1, is_get,
           "???", url,
           MALFORMED, version,
           -1, keep_alive);

    strncpy(request_string, "GET /hang/on/I'm/not/done/with/this/lin", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, url, &version, &keep_alive);
    printf("---------case 11---------\n"
           "expected  :  actual\n"
           "return: %d : %d\n"
           "GET: %d : %d\n"
           "URL: %s : %s\n"
           "version: %d : %d\n"
           "keep alive: %d : %d\n"
            , FAIL, result,
           -1, is_get,
           "???", url,
           -1, version,
           -1, keep_alive);

    return 0;
}

int main(int argc, char* argv[]) {
    return test_parse_request_string();
}