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

    strncpy(request_string, "GET http://host.name.funky.port:2684/file.jpg HTTP/1.1\r\n", JOB_REQUEST_BUFFER_SIZE);
    result = parse_request_string(request_string ,&is_get, hostname, url,  &version, &keep_alive);
    printf("---------case 5---------\n"
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
           "host.name.funky.port:2684", hostname,
           DOT_ONE, version,
           false, keep_alive);

    printf("\n\n\n\n");
    return 0;
}

int test_parse_response_string() {
    char response_string[JOB_REQUEST_BUFFER_SIZE + 1];
    int response_header_length;
    int response_content_length;

    strncpy(response_string,
            "HTTP/1.1 200 OK\r\n"
            "Date: Sat, 09 Apr 2022 20:22:04 GMT\r\n"
            "Server: Apache/2.4.6 (CentOS) OpenSSL/1.0.2k-fips mod_fcgid/2.3.9 PHP/5.4.16\r\n"
            "Last-Modified: Wed, 13 Nov 2019 17:15:57 GMT\r\n"
            "ETag: \"d14-5973d84996d40\"\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: 3348\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "\r\n"
            "<head>\n"
            "<link rel=\"shortcut icon\" href=\"graphics/html.gif\" type=\"image/gif\" />\n"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />\n"
            "<meta name=\"description\" content=\"Website short description.\" />\n"
            "<meta name=\"keywords\" content=\"website main keywords\" />\n"
            "\t<title>Welcome to SHS</title>\n"
            "<script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js\"></script>\n"
            "<script>\n"
            "\t!window.jQuery && document.write('<script src=\"jquery-1.4.3.min.js\"><\\/script>');\n"
            "</script>\n"
            "<script type=\"text/javascript\" src=\"./fancybox/jquery.mousewh"
            , JOB_REQUEST_BUFFER_SIZE);
    parse_response_header(response_string, &response_header_length, &response_content_length);
    printf("---------case 0---------\n"
           "header-length %d/%ld\n"
           "content-length %d/%d\n",
           response_header_length, strlen("HTTP/1.1 200 OK\r\nDate: Sat, 09 Apr 2022 20:22:04 GMT\r\nServer: Apache/2.4.6 (CentOS) OpenSSL/1.0.2k-fips mod_fcgid/2.3.9 PHP/5.4.16\r\nLast-Modified: Wed, 13 Nov 2019 17:15:57 GMT\r\nETag: \"d14-5973d84996d40\"\r\nAccept-Ranges: bytes\r\nContent-Length: 3348\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"),
           response_content_length, 3348);

    strncpy(response_string,
            "HTTP/1.1 404 Not Found\r\n"
            "Date: Sat, 09 Apr 2022 20:14:25 GMT\r\n"
            "Server: Apache/2.4.6 (CentOS) OpenSSL/1.0.2k-fips mod_fcgid/2.3.9 PHP/5.4.16\r\n"
            "Content-Length: 214\r\n"
            "Content-Type: text/html; charset=iso-8859-1\r\n"
            "\r\n"
            "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
            "<html><head>\n"
            "<title>404 Not Found</title>\n"
            "</head><body>\n"
            "<h1>Not Found</h1>\n"
            "<p>The requested URL /files/done.exist was not found on this server.</p>\n"
            "</body></html>\n"
            "\r\n"
            , JOB_REQUEST_BUFFER_SIZE);
    parse_response_header(response_string, &response_header_length, &response_content_length);
    printf("---------case 1---------\n"
           "total-length: %d/%d\n"
           "header-length %d/%d\n"
           "content-length %d/%d\n"
           ,response_content_length + response_header_length,421,
           response_header_length, 421 - 214,
           response_content_length, 214);

    strncpy(response_string,
            "HTTP/1.1 404 Not Found\r\n"
            "Date: Sat, 09 Apr 2022 19:36:17 GMT\r\n"
            "Server: Apache/2.4.6 (CentOS) OpenSSL/1.0.2k-fips mod_fcgid/2.3.9 PHP/5.4.16\r\n"
            , JOB_REQUEST_BUFFER_SIZE);
    parse_response_header(response_string, &response_header_length, &response_content_length);
    printf("---------case 2---------\n"
           "header-length %d/%ld\n"
           "content-length %d/%d\n",
           response_header_length, strlen(response_string),
           response_content_length, -1);

    strncpy(response_string,
            "Content-Length: 9\r\n\r\n123456789"
            , JOB_REQUEST_BUFFER_SIZE);
    parse_response_header(response_string, &response_header_length, &response_content_length);
    printf("---------case 3---------\n"
           "header-length %d/%d\n"
           "content-length %d/%d\n",
           response_header_length, 25,
           response_content_length, 11);

    strncpy(response_string,
            "HTTP/1.1 200 OK\r\n"
            "Date: Sat, 09 Apr 2022 20:09:07 GMT\r\n"
            "Server: Apache/2.4.6 (CentOS) OpenSSL/1.0.2k-fips mod_fcgid/2.3.9 PHP/5.4.16\r\n"
            "Last-Modified: Tue, 01 Sep 2015 15:49:52 GMT\r\n"
            "ETag: \"29-51eb1802c8800\"\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: 41\r\n"
            "Content-Type: text/plain; charset=UTF-8\r\n"
            "\r\n"
            "This is a text File. Welcome to My Server"
            , JOB_REQUEST_BUFFER_SIZE);
    parse_response_header(response_string, &response_header_length, &response_content_length);
    printf("---------case 4---------\n"
           "total-length: %d/%d\n"
           "header-length %d/%d\n"
           "content-length %d/%d\n",
           response_content_length + response_header_length,330,
           response_header_length, 330 - 41,
           response_content_length, 41);
    printf("\n\n\n\n");
    return 0;
}

int main(int argc, char* argv[]) {
    return test_parse_request_string() + test_parse_response_string();
}
