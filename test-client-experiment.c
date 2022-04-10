//
// Created by dran on 4/9/22.
//
//
// let's try and send predetermined string to server
// Created by dran on 2/2/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define DEBUG 1
#define SUCCESS 0
#define FAIL 1


struct message {
    int num;
    char str[64];
};

int main(int argc, char* argv[]) {
    int ret_status;
    int socket_fd;
    struct addrinfo server_address_hints;
    struct addrinfo* server_address;
    char buffer[MAX_BUFFER_SIZE];

    if (argc != 4) {
        printf("usage: %s <servername> <port> <path>\n", argv[0]);
        return 1;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("error: unable to create socket\n");
        return 1;
    }

    server_address_hints.ai_family = AF_INET;
    server_address_hints.ai_socktype = SOCK_STREAM;
    ret_status = getaddrinfo(argv[1], argv[2], &server_address_hints, &server_address);
    if (ret_status != 0) {
        printf("resolved address status: %d error:%s\n", ret_status, strerror(errno));
        return 1;
    }

    ret_status = connect(socket_fd, server_address->ai_addr, server_address->ai_addrlen);
    printf("connected : %d\n", socket_fd);
    sprintf(buffer, "GET %s HTTP/1.1\r\n\r\n", argv[3]);
    ret_status = send(socket_fd, buffer, strlen(buffer), 0);
    if (ret_status == -1) {
        printf("error: failed to send to server\n");
        return 1;
    } else {
        memset(buffer, 0, MAX_BUFFER_SIZE);
    }
    printf("request sent: %d\n%s\n",ret_status, buffer);

    ret_status = recv(socket_fd, buffer, MAX_BUFFER_SIZE, 0);
    if (ret_status == -1) {
        printf("error: failed to receive from server\n");
        return 1;
    } else {
        buffer[ret_status] = '\0';
        printf("echoed from server:\nbytes:%d\n%s\n", ret_status, buffer);
    }

    freeaddrinfo(server_address);
    close(socket_fd);
    printf("congrats!\n");
    return 0;
}

