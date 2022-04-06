//
// Created by dran on 4/5/22.
//

#ifndef NS_PA_2_CACHE_RECORD_H
#define NS_PA_2_CACHE_RECORD_H

#include <pthread.h>
#include "uthash.h"

#define CACHE_HASH_LENGTH 33  // 32 bytes for md5sum output

extern unsigned int CACHE_TTL;

enum action_status {unavailable, should_read, should_write};

struct cache_record {
    char name[CACHE_HASH_LENGTH];
    unsigned long expiration_time;
    int write_in_progress;
    int readers;
    pthread_mutex_t mutex;
    UT_hash_handle hh;
};
typedef struct cache_record cache_record_t;

struct cache_record_table {
    cache_record_t* content;
    pthread_mutex_t mutex;
};
typedef struct cache_record_table cache_table_t;

cache_record_t* cache_record_create(char* name);
cache_table_t* cache_table_create();

int cache_record_get_or_create(cache_table_t* table, char* url, cache_record_t** record_ret, enum action_status* status_ret);
int cache_record_close(cache_record_t* record, enum action_status status);




#endif //NS_PA_2_CACHE_RECORD_H
