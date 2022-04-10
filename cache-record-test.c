//
// Created by dran on 4/6/22.
//

#include <stdio.h>
#include <unistd.h>

#include "cache-record.h"

int CACHE_TTL;

int test_cache_record () {
    cache_table_t* table;
    cache_record_t* record_a;
    cache_record_t* record_b;
    cache_record_t* record_c;
    enum action_status status;

    CACHE_TTL = 1;

    table = cache_table_create();

    record_a = cache_record_get_or_create(table, "fake.url.com/first.cached.file", &status);
    printf("after first record creation, record name : %s status: %d/%d\n", record_a->name, status, should_write);

    record_b = cache_record_get_or_create(table, "fake.url.com/first.cached.file", &status);
    printf("record found but is being written to, record name : %s status: %d/%d\n", record_b->name, status, unavailable);

    cache_record_close(record_a, should_write);
    record_c = cache_record_get_or_create(table, "fake.url.com/first.cached.file", &status);
    printf("write finished available for readers, record name : %s status: %d/%d\n", record_c->name, status, should_read);

    record_b = cache_record_get_or_create(table, "fake.url.com/first.cached.file", &status);
    printf("multiple readers are allowed the same file, record name : %s status: %d/%d\n", record_b->name, status, should_read);

    // read fished for both readers
    cache_record_close(record_b, should_read);
    cache_record_close(record_c, should_read);

    sleep(2);
    printf("record state: readers:%d writing:%d\n", record_c->readers, record_c->write_in_progress);

    record_a = cache_record_get_or_create(table, "fake.url.com/first.cached.file", &status);
    printf("cache is now stale, should update it, record name : %s status: %d/%d\n", record_a->name, status, should_write);
    cache_record_close(record_a, should_write);

    return 0;
}


int main(int arc, char* argv[]) {
    return test_cache_record();
}
