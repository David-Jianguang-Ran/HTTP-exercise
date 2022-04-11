//
// Created by dran on 3/12/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "worker.h"

#define DEBUG 1

int SHOULD_SHUTDOWN;

void shutdown_signal_handler(int sig_num) {
    SHOULD_SHUTDOWN = 1;
}

// this function is not thread safe
int load_block_list(block_table_t* in_memory_table) {
    FILE* block_list_file;
    char block_list_entry[MAX_URL_SIZE + 1];
    int result;
    char* result_ptr;
    char* new_line_char;

    block_list_file = fopen(BLOCK_LIST_PATH, "r");
    if (block_list_file == NULL) {
        printf("blocklist not found. proceeding with empty block table\n");
        return FAIL;
    }

    result_ptr = fgets(block_list_entry, MAX_URL_SIZE, block_list_file);
    while (result_ptr != NULL) {
        new_line_char = strchr(block_list_entry, '\n');
        if (new_line_char != NULL) {
            *new_line_char = '\0';
        }
        result = block_table_add(in_memory_table, block_list_entry);
        if (DEBUG && (result == SUCCESS)) {
            printf("blocked entry: %s\n", block_list_entry);
        }
        result_ptr = fgets(block_list_entry, MAX_URL_SIZE, block_list_file);
    }
    fclose(block_list_file);
    return SUCCESS;
}

int main(int argc, char* argv[]) {
    int ret_status;

    if (argc < 2) {
        printf("usage: %s <port> <cache-timeout>\n", argv[0]);
        return 1;
    }

    if (argc == 3) {
        CACHE_TTL = atoi(argv[2]);
    }

    // custom shutdown signal handling
    struct sigaction new_action;

    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = shutdown_signal_handler;
    sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGINT, &new_action, NULL);

    // create main listener socket
    int listener_socket_fd;
    int connected_fd;
    struct sockaddr_in server_sock;
    socklen_t server_sock_len;

    listener_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_socket_fd == -1) {
        printf("error: unable to create socket\n");
        return 1;
    }
    server_sock.sin_family = AF_INET;
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sock.sin_port = htons(atoi(argv[1]));
    server_sock_len = sizeof(struct sockaddr_in);
    ret_status = bind(listener_socket_fd, (struct sockaddr *) &server_sock, server_sock_len);
    if (ret_status != SUCCESS) {
        printf("error: unable to bind to port %d", atoi(argv[1]));
        return 1;
    }

    // spawn workers
    pthread_t workers[CLIENT_WORKER_THREADS + PREFETCH_WORKER_THREADS];
    struct resource_info worker_resource[CLIENT_WORKER_THREADS + PREFETCH_WORKER_THREADS];
    job_t* new_job;
    int i;

    worker_resource[0] = create_shared_resource(CLIENT_JOB_STACK_SIZE, CLIENT_WORKER_THREADS);
    load_block_list(worker_resource[0].block_table);  // must load blocked list before spawning worker threads

    for (i = 0; i < CLIENT_WORKER_THREADS; i++) {
        worker_resource[i] = worker_resource[0];
        worker_resource[i].thread_id = i + 100;
        worker_resource[i].is_client_worker = 1;
        pthread_create(&workers[i], NULL, worker_main, &worker_resource[i]);
    }
    for (i = CLIENT_WORKER_THREADS; i < PREFETCH_WORKER_THREADS + CLIENT_WORKER_THREADS; i++) {
        worker_resource[i] = worker_resource[0];
        worker_resource[i].thread_id = i + 200;
        worker_resource[i].is_client_worker = 0;
        pthread_create(&workers[i], NULL, worker_main, &worker_resource[i]);
    }

    // listen for incoming connection
    SHOULD_SHUTDOWN = 0;
    ret_status = listen(listener_socket_fd, CLIENT_JOB_STACK_SIZE);
    safe_write(worker_resource[0].std_out, "Server listening for connection\n");
    while (!SHOULD_SHUTDOWN) {
        connected_fd = accept(listener_socket_fd, NULL, NULL);
        new_job = job_construct(connected_fd);
        ret_status = FAIL;
        while (ret_status == FAIL) {
            ret_status = job_stack_push(worker_resource[0].client_job_stack, new_job);
        }
    }

    // shutdown routine
    safe_write(worker_resource[0].std_out, "Shutdown signal received\n");
    job_stack_signal_finish(worker_resource[0].client_job_stack);
    job_stack_signal_finish(worker_resource[0].prefetch_job_stack);
    for (i = 0; i < CLIENT_WORKER_THREADS + PREFETCH_WORKER_THREADS; i++) {
        pthread_join(workers[i], NULL);
    }

    close(listener_socket_fd);
    safe_write(worker_resource[0].std_out, "Shutdown complete. Good Bye!\n");
    free_shared_resource(&worker_resource[0]);
    return 0;
}