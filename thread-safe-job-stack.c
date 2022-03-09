//
// Created by dran on 3/5/22.
//

#include <stdlib.h>

#include "thread-safe-job-stack.h"


job_stack_t* job_stack_construct(unsigned int max_jobs, unsigned int reserve_slots) {
    job_stack_t* created;
    created = malloc(sizeof(job_stack_t));
    if (created == NULL) {
        return created;
    }
    created->content_base = malloc(max_jobs * sizeof(job_t));
    pthread_cond_init(&(created->not_empty), NULL);
    pthread_cond_init(&(created->not_full), NULL);
    pthread_mutex_init(&(created->mutex), NULL);
    created->finished = false;
    created->max_job_count = max_jobs;
    created->reserve_job_count = reserve_slots;
    created->top = 0;
    return created;
}

void job_stack_destruct(job_stack_t* to_free) {
    free(to_free->content_base);
    pthread_mutex_destroy(&(to_free->mutex));
    pthread_cond_destroy(&(to_free->not_full));
    pthread_cond_destroy(&(to_free->not_empty));
    free(to_free);
}

int job_stack_push(job_stack_t* stack, job_t* ptr_in) {
    if (stack->finished) {
        return FAIL;
    }
    pthread_mutex_lock(&(stack->mutex));
    // if stack has more than max - reserve items,
    // wait until a consumer has popped some off stack and try again
    if (stack->top >= (stack->max_job_count - stack->reserve_job_count)) {
        pthread_cond_wait(&(stack->not_full), &(stack->mutex));
        pthread_mutex_unlock(&(stack->mutex));  // TODO : is there a way to avoid unlock than lock again in recursive call?
        return job_stack_push(stack, ptr_in);
    }
    job_stack_get_item(stack, stack->top) = ptr_in;
    stack->top++;
    pthread_cond_signal(&(stack->not_empty));
    pthread_mutex_unlock(&(stack->mutex));
    return SUCCESS;
}

int job_stack_pop(job_stack_t* stack, job_t** ptr_out) {
    pthread_mutex_lock(&(stack->mutex));
    // unless finished, wait until some job has been pushed onto stack and try to pop again
    if (stack->top == 0 && !stack->finished) {
        pthread_cond_wait(&(stack->not_empty), &(stack->mutex));
        pthread_mutex_unlock(&(stack->mutex));
        return job_stack_pop(stack, ptr_out);
    } else if (stack->top == 0) {
        pthread_mutex_unlock(&(stack->mutex));
        return FINISHED;
    }
    *ptr_out = job_stack_get_item(stack, stack->top);
    stack->top--;
    pthread_cond_signal(&(stack->not_empty));
    pthread_mutex_unlock(&(stack->mutex));
    return SUCCESS;
}

int job_stack_signal_finish(job_stack_t* stack) {
    pthread_mutex_lock(&(stack->mutex));
    stack->finished = true;
    pthread_cond_broadcast(&(stack->not_empty));
    pthread_mutex_unlock(&(stack->mutex));
    return SUCCESS;
}

job_t* job_stack_get_item(job_stack_t* stack, unsigned int index) {
    return *(stack->content_base + index)
}