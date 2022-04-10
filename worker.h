//
// Created by dran on 3/8/22.
//

#ifndef NS_PA_2_WORKER_H
#define NS_PA_2_WORKER_H

#include <stdbool.h>
#include <netdb.h>

#include "parsing.h"
#include "block-table.h"
#include "cache-record.h"
#include "job.h"
#include "thread-safe-file.h"
#include "thread-safe-job-stack.h"

#include "constants.h"


struct resource_info {
    int thread_id;
    // int is_client_worker;
    block_table_t* block_table;
    cache_table_t* cache_table;
    job_stack_t* client_job_stack;
    // job_stack_t* cache_job_stack;
    pthread_mutex_t* addr_lookup_lock;  // used for calling getaddrinfo safely
    safe_file_t* std_out;
};

enum host_status {ok, error, not_found, blocked};

// thread_id of returned is always -1 for main thread
// copy and change to appropriate id before passing to workers
// resource info is safe to copy and pass around
struct resource_info create_shared_resource(int job_stack_size, int reserve_slots);
// all worker threads MUST BE finished (joined) before freeing resource
// calling this frees all underlying resource, only need to call it once for multiple copies
void free_shared_resource(struct resource_info* ptr_to_resource);

void* worker_main(void* shared);

// this function:
// - gets a request from client
// - checks host name against block table
// - looks up address for host name
// - checks resolved address against block table
// - validates reqeust
// - checks request against cache record
// - << if cache miss >>
// - opens connection to server and send request over
// - relays server response to client cache the file response from server
// - << if cache hit >>
// - responds with cached file
int process_job(job_t* current_job, struct resource_info* shared_resource);
// int process_prefetch_job(job_t* current_job, struct resource_info* shared_resource);



// resolves hostname to address
// only one thread at a time using 'shared_resource.addr_lookup_lock'
// checks both hostname and address against `shared_resource.block_table`
// any resolved addresses NOT in block table will count as ok
enum host_status resolve_host(struct resource_info* shared_resource, char* hostname, struct addrinfo** server_address);
// respond to client either from cache or web server
// may modify cache
int handle_valid_request(job_t* current_job, struct resource_info* shared_resource, struct addrinfo* server_address,
                        char* path, char* hostname, char* response_buffer, int* response_tail, char* output_buffer);

// this function is for getting the response, saving response body to cache, and sending to client
// this function expects a complete response waiting in server_socket
// will close the cache file if a fail is thrown inside this function
int cache_and_send(int client_socket, FILE* cache_file,char* buffer, int length);

// this function DOES NOT check str boundary
void copy_into_buffer(char* target, int* target_tail, char* content);
// returns SUCCESS after all up to length is sent
// returns FAIl if connection is already closed
int try_send_in_chunks(int socket_fd, char* buffer, int length);


#endif //NS_PA_2_WORKER_H
