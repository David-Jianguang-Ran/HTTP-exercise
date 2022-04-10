//
// Created by dran on 3/5/22.
//

#ifndef NS_PA_2_JOB_H
#define NS_PA_2_JOB_H

#include "constants.h"

// why use struct instead of just a socket_fd?
// using a struct makes it easier to extend and add `connection: keep-alive` feature
struct job_info {
    int client_socket_fd;
    char request[JOB_REQUEST_BUFFER_SIZE + 1];
    int request_tail;
    unsigned int expiration_time;
};
typedef struct job_info job_t;

job_t* job_construct(int socket_fd);
// will close client_socket_fd
void job_destruct(job_t*);
void job_clear_buffer(job_t*);

#endif //NS_PA_2_JOB_H
