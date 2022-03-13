//
// Created by dran on 3/8/22.
//

#ifndef NS_PA_2_WORKER_H
#define NS_PA_2_WORKER_H

#include <stdbool.h>

#include "job.h"
#include "thread-safe-file.h"
#include "thread-safe-job-stack.h"

#define TERMINATE 10  // should these be an enum?
#define ENQUEUE 11
#define DEBUG 1

#define KEEP_ALIVE_TIMEOUT 10
#define DOCUMENT_ROOT "./www"


struct resource_info {
    int thread_id;
    job_stack_t* job_stack;
    safe_file_t* std_out;
};

enum http_version {MALFORMED, NOT_SUPPORTED, DOT_ZERO, DOT_ONE};

// thread_id of returned is always -1 for main thread
// copy and change to appropriate id before passing to workers
// resource info is safe to copy and pass around
struct resource_info create_shared_resource(int job_stack_size, int reserve_slots);
// all worker threads MUST BE finished (joined) before freeing resource
// calling this frees all underlying resource, only need to call it once for multiple copies
void free_shared_resource(struct resource_info* ptr_to_resource);
void* worker_main(void* shared);
int process_job(job_t* current_job, struct resource_info* resource);
// request string must be null terminated or face UNDEFINED CONSEQUENCES!!
// will remove trailing / from url, unless it's only a slash
int parse_request_string(char* request_string, bool* is_GET, char* url, enum http_version* version, bool* keep_alive);
// simple wrapper for strncmp, only match up to strlen(command)
// all strings must be zero terminated
// return 0 for no match, 1 for match
int matches_command(char* target, char* command);
int matches_command_case_insensitive(char* target, char* command);
// this function trys to look up the requested resource
// and fill appropriate response into buffer
// including content type and content length
// will try to add .html or .htm to url if url is an directory
// if not found or forbidden, a nullptr will be returned
// else an open file
FILE* fill_content_info(char* out_going_buffer, int* out_going_buffer_tail, char* url);
// this function DOES NOT check str boundary
void copy_into_buffer(char* target, int* target_tail, char* content);
// returns SUCCESS after all up to length is sent
// returns FAIl if connection is already closed
int try_send_in_chunks(int socket_fd, char* buffer, int length);


#endif //NS_PA_2_WORKER_H
