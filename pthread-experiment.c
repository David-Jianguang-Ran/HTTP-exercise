//
// Created by dran on 3/11/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>


#define THREADS 5


int GLOBAL_VAR;
pthread_mutex_t VAR_LOCK;
struct timespec DELAY;

struct heap_obj {
    int var;
    pthread_mutex_t mutex;
};

void* thread_main(void* arg) {
    int i;
    struct heap_obj* shared_obj;

    shared_obj = (struct heap_obj* )arg;

    while (i < THREADS) {
        pthread_mutex_lock(&(shared_obj->mutex));
        printf("t shared: %d\n", shared_obj->var);
        pthread_mutex_unlock(&(shared_obj->mutex));
        nanosleep(&DELAY, NULL);
        i++;
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int i;
    pthread_t threads[THREADS];
    struct heap_obj* shared_obj;

    srand(time(NULL));
    pthread_mutex_init(&VAR_LOCK, NULL);

    shared_obj = malloc(sizeof(struct heap_obj));
    DELAY.tv_sec = 0;
    DELAY.tv_nsec = 10000;

    shared_obj->var = rand() % 1024;
    pthread_mutex_init(&(shared_obj->mutex), NULL);

    for (i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_main, shared_obj);
    }

    for (i = 0; i < THREADS; i++) {
        pthread_mutex_lock(&(shared_obj->mutex));
        shared_obj->var++;
        printf("m shared: %d\n", shared_obj->var);
        pthread_mutex_unlock(&(shared_obj->mutex));
        nanosleep(&DELAY, NULL);
    }


    for (i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;

}