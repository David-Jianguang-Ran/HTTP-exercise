//
// Created by dran on 3/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>

#define DEBUG 1

#include "thread-safe-job-stack.h"
#include "thread-safe-file.h"

#define THREADS 5
#define JOBS 10

struct heap_obj {
    int var;
    pthread_mutex_t mutex;
};

void* thread_main(void* arg) {
    job_stack_t* job_stack;
    job_t* current_job;
    int ret_status;

    job_stack = (job_stack_t* )arg;

    while (1) {
        ret_status = job_stack_pop(job_stack, &current_job);
        if (ret_status == FINISHED) {
            return NULL;
        } else if (ret_status == SUCCESS) {
            job_destruct(current_job);
        }
    }
}

int main(int argc, char* argv[]) {
    int i;
    pthread_t threads[THREADS];
    int ret_status;
    job_stack_t* job_stack;
    job_t* new_job;

    srand(time(NULL));

    job_stack = job_stack_construct(JOBS / 4,THREADS);


    for (i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_main, job_stack);
    }

    for (i = 0; i < JOBS; i++) {
        new_job = job_construct(i + 100);
        ret_status = FAIL;
        while (ret_status == FAIL) {
            ret_status = job_stack_push(job_stack, new_job);
        }
    }

    ret_status = job_stack_signal_finish(job_stack);

    for (i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    job_stack_destruct(job_stack);
    return 0;

}