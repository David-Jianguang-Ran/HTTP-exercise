//
// Created by dran on 4/5/22.
//

#include <stdio.h>

#include "block-table.h"


int single_threaded_add_check_test () {
    block_table_t* table;
    int result;

    table = block_table_create();

    block_table_add(table, "key1");
    block_table_add(table, "key2");
    block_table_add(table, "key3");

    result = block_table_check(table, "key1");
    printf("key1, found %d/%d\n", result, 1);

    result = block_table_check(table, "key2");
    printf("key2, found %d/%d\n", result, 1);

    result = block_table_check(table, "key3");
    printf("key3, found %d/%d\n", result, 1);

    result = block_table_check(table, "key4");
    printf("key4, found %d/%d\n", result, 0);

    return 0;
}

int main (int argc, char argv[]) {
    return single_threaded_add_check_test();
}