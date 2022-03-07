//
// Created by dran on 3/5/22.
//

#ifndef NS_PA_2_THREAD_SAFE_JOB_STACK_H
#define NS_PA_2_THREAD_SAFE_JOB_STACK_H

#include <pthread.h>
#include "job.h"

#define FINISHED -1
#define SUCCESS 0
#define FAIL 1


struct job_stack {
    job_t** content_base;
    unsigned int top;
    unsigned int max_job_count;
    unsigned int reserve_job_count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int finished;
};
typedef struct job_stack job_stack_t;


// the # of reserve_slots must be greater than # of consumers that also want to produce sometimes
job_stack_t* job_stack_construct(unsigned int max_jobs, unsigned int reserve_slots);
// job stack MUST BE empty before calling destruct
void job_stack_destruct(job_stack_t* to_free);

// this function is used for push new jobs onto stack, should be used by job producers
// will block until stack has less than (max - reserve) items
int job_stack_push(job_stack_t* stack, job_t* ptr_in);
// this function should be used by job consumers that sometimes have to produce too
// this function is for pushing existing jobs back onto stack
// pushes to the bottom of stack
int job_stack_push_back(job_stack_t* stack, job_t* ptr_in);
int job_stack_pop(job_stack_t* stack, job_t** ptr_out);
int job_stack_signal_finish(job_stack_t* stack);

job_t* job_stack_get_item(job_stack_t* stack, unsigned int index);

#endif //NS_PA_2_THREAD_SAFE_JOB_STACK_H
