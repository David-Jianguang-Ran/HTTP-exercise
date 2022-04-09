//
// Created by dran on 4/5/22.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "cache-record.h"

#define SUCCESS 0
#define FAIL 1

#define URL_MAX_LENGTH 2048

cache_table_t* cache_table_create() {
    cache_table_t* created;

    created = malloc(sizeof(cache_table_t));
    created->content = NULL;
    pthread_mutex_init(&(created->mutex), NULL);
    return created;
}

cache_record_t* cache_record_create(char* name) {
    cache_record_t* created;

    created = malloc(sizeof(cache_record_t));
    strcpy(created->name, name);
    created->expiration_time = time(NULL) + CACHE_TTL;
    created->write_in_progress = 1;
    created->readers = 0;
    pthread_mutex_init(&(created->mutex), NULL);
    return created;
}

cache_record_t* cache_record_get_or_create(cache_table_t* table, char* url, enum action_status* status_ret) {
    char hash_command[URL_MAX_LENGTH + 256];
    FILE* hash_return;
    char hash_buffer[CACHE_HASH_LENGTH];
    cache_record_t* record;

    record = NULL;
    if (strlen(url) > URL_MAX_LENGTH) {
        *status_ret = unavailable;
        return record;  // unable to process url longer than 2048 bytes
    }

    sprintf(hash_command, "md5sum <<< '%s'", url);  // TODO:  this looks like a remote code execution vulnerability. find out for sure
    // is this a valid url? : " | echo nasty-command" maybe as hexadecimal?
    hash_return = popen(hash_command, "r");
    fgets(hash_buffer, 32, hash_return);
    hash_buffer[32] = '\0';
    fclose(hash_return);

    pthread_mutex_lock(&(table->mutex));
    HASH_FIND_STR(table->content, hash_buffer, record);
    if (record == NULL) {
        // new cache record is created, it should be filled with actual data
        record = cache_record_create(hash_buffer);
        HASH_ADD_STR(table->content, name, record);
        *status_ret = should_write;
        pthread_mutex_unlock(&(table->mutex));
        return record;
    }

    pthread_mutex_lock(&(record->mutex));
    if (record->expiration_time < time(NULL) && !record->readers && !record->write_in_progress) {
        // if cache is stale and no one else is updating it, update it and reset TTL
        *status_ret = should_write;
        record->write_in_progress = 1;
        record->expiration_time = time(NULL) + CACHE_TTL;
    } else if (record->expiration_time > time(NULL) && !record->write_in_progress) {
        // if cache is fresh enough and no writer is currently updating it, reading is ok
        *status_ret = should_read;
        record->readers++;
    } else {
        // cache is expired, don't read.
        // someone else is still using cache (I hope they started before expiration), don't update
        // someone else is already updating it, don't update
        *status_ret = unavailable;
    }
    pthread_mutex_unlock(&(record->mutex));

    pthread_mutex_unlock(&(table->mutex));
    return record;
}

void cache_record_close(cache_record_t* record, enum action_status status) {
    pthread_mutex_lock(&(record->mutex));
    if (status == should_write) {
        record->write_in_progress = 0;
    } else if (status == should_read) {
        record->readers--;
    }
    pthread_mutex_unlock(&(record->mutex));
}