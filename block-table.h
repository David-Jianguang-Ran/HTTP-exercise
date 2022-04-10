//
// Created by dran on 4/4/22.
//

#ifndef NS_PA_2_BLOCK_TABLE_H
#define NS_PA_2_BLOCK_TABLE_H

#include <pthread.h>
#include "uthash.h"

#define MAX_NAME_LENGTH 256

struct table_element {
    char name[MAX_NAME_LENGTH];
    UT_hash_handle hh;
};

struct block_table {
    struct table_element* content;
    pthread_mutex_t mutex;
};
typedef struct block_table block_table_t;

// this object will not be freed until shutdown by OS
block_table_t* block_table_create();

// the functions below are threadsafe
int block_table_add(block_table_t* table, char* name);
// this function return 0 if name is not found in table
// return 1 if name has been added
int block_table_check(block_table_t* table, char* name);

#endif //NS_PA_2_BLOCK_TABLE_H
