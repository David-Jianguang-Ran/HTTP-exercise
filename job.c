//
// Created by dran on 3/5/22.
//

#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include "job.h"

job_t* job_construct(int socket_fd) {
    job_t* created;
    created = malloc(sizeof(job_t));
    created->socket_fd = socket_fd;
    job_clear_buffer(created);
    created->expiration_time = 0;
    return created;
}

void job_destruct(job_t* target) {
    close(target->socket_fd);
    free(target);
}

void job_clear_buffer(job_t* target) {
    memset(target->request, 0, JOB_REQUEST_BUFFER_SIZE + 1);
    target->request_tail = 0;
}