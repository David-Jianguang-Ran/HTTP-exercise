//
// Created by dran on 4/5/22.
//

#ifndef NS_PA_2_CACHE_RECORD_H
#define NS_PA_2_CACHE_RECORD_H

#include <pthread.h>
#include "uthash.h"

#include "constants.h"

extern int CACHE_TTL;

enum action_status {unavailable, should_read, should_write};

struct cache_record {
    char name[CACHE_HASH_LENGTH];
    long expiration_time;
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

// cache_record_t is a "reader writer problem" type object,
// other than reading `name` please treat it like an opaque object
// please don't modify cache_record_t on your own
cache_record_t* cache_record_create(char* name);
cache_table_t* cache_table_create();

// will create and return pointer to a cache_record_t
// uses arg url as key (md5 hashed and stored in `cache_record_t.name`)
// if you get a should_write it means you are the sole authorized writer, you must update the file than close the record
// MUST call cache_record_close with the action status code returned here to release cache_record_t
cache_record_t* cache_record_get_or_create(cache_table_t* table, char* url, enum action_status* status_ret);
void cache_record_close(cache_record_t* record, enum action_status status);




#endif //NS_PA_2_CACHE_RECORD_H
