//
// Created by dran on 4/4/22.
//

#include <stdlib.h>
#include <strings.h>
#include "block-table.h"

#define SUCCESS 0
#define FAIL 1

block_table_t* block_table_create() {
    block_table_t* created;

    created = malloc(sizeof(block_table_t));
    created->content = NULL;
    pthread_mutex_init(&(created->mutex), NULL);
    return created;
}

int block_table_add(block_table_t* table, char* name) {
    struct table_element* new_element;
    struct table_element* found_element;

    new_element = malloc(sizeof(struct table_element));
    strncpy(new_element->name, name, MAX_NAME_LENGTH - 1);
    pthread_mutex_lock(&(table->mutex));
        HASH_FIND_STR(table->content, name, found_element);  // uthash table must have unique entries
        if (found_element == NULL) {
            HASH_ADD_STR(table->content, name, new_element);
        } else { // quietly ignore duplicate entries
            free(new_element);
        }
    pthread_mutex_unlock(&(table->mutex));
    return SUCCESS;
}

int block_table_check(block_table_t* table, char* name) {
    struct table_element* found_element;

    pthread_mutex_lock(&(table->mutex));
        HASH_FIND_STR(table->content, name, found_element);
    pthread_mutex_unlock(&(table->mutex));
    if (found_element == NULL) {
        return 0;
    } else {
        return 1;
    }
}