//
// Created by dran on 4/6/22.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    char buffer_len;
    struct message* my_message;

    if (argc != 3) {
        printf("usage: %s <servername> <port>\n", argv[0]);
        return 1;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("error: unable to create socket\n");
        return 1;
    }

    server_address_hints.ai_family = AF_UNSPEC;
    server_address_hints.ai_socktype = SOCK_STREAM;
    server_address_hints.ai_flags = AI_PASSIVE;
    ret_status = getaddrinfo(argv[1], argv[2], &server_address_hints, &server_address);
    if (ret_status != 0) {
        printf("error: failed to get server address info\n");
        return 1;
    }


    freeaddrinfo(server_address);
    close(socket_fd);
    printf("congrats!\n");
    return 0;
}