//
// Created by dran on 3/7/22.
//



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 1024
#define DEBUG 1
#define SUCCESS 0
#define FAIL 1

int main(int argc, char* argv[]) {
    int ret_status;
    int listener_socket_fd;
    int connected_fd;
    struct sockaddr_in client_sock;
    socklen_t client_sock_len;
    struct sockaddr_in server_sock;
    socklen_t server_sock_len;
    char buffer[MAX_BUFFER_SIZE + 1];
    char* char_ptr;

    buffer[MAX_BUFFER_SIZE] = '\0';

    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        return 1;
    }

    listener_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_socket_fd == -1) {
        printf("error: unable to create socket\n");
        return 1;
    }

    server_sock.sin_family = AF_INET;
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sock.sin_port = htons(atoi(argv[1]));
    server_sock_len = sizeof(struct sockaddr_in);

    ret_status = bind(listener_socket_fd, (struct sockaddr*)&server_sock, server_sock_len);
    if (ret_status != SUCCESS) {
        printf("error: unable to bind to port %d", atoi(argv[1]));
        return 1;
    }
    ret_status = listen(listener_socket_fd, 10);

    if (DEBUG) printf("debug: server started\n");

    while (1) {
        client_sock_len = sizeof(struct sockaddr_in);
        connected_fd = accept(listener_socket_fd, (struct sockaddr*)&client_sock, &client_sock_len);
        if (connected_fd != -1) {
            ret_status = recv(connected_fd, buffer, MAX_BUFFER_SIZE, 0);
            printf("NULL at %p\n",NULL);
            char_ptr = strchr(buffer, '\r');
            printf("CR at %p\n",char_ptr);
            char_ptr = strchr(buffer, '\n');
            printf("LF at %p\n",char_ptr);
            buffer[ret_status] = '\0';
            printf("message received bytes:%d:\n%s\n",ret_status, buffer);
            close(connected_fd);
        }
    }
}
