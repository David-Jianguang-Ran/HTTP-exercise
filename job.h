//
// Created by dran on 3/5/22.
//

#ifndef NS_PA_2_JOB_H
#define NS_PA_2_JOB_H

#define JOB_TIME_TO_LIVE 10
#define JOB_REQUEST_BUFFER_SIZE 4096

// why use struct instead of just a socket_fd?
// using a struct makes it easier to extend and add `connection: keep-alive` feature
// TODO : maybe i should put a request char buffer here, this way slow loris requests can just be popped back onto the stack for later
struct job_info {
    int socket_fd;
    char request[JOB_REQUEST_BUFFER_SIZE + 1];
    int request_tail;
    unsigned int expiration_time;
};
typedef struct job_info job_t;

job_t* job_construct(int socket_fd);
void job_destruct(job_t*);
void job_clear_buffer(job_t*);

#endif //NS_PA_2_JOB_H
