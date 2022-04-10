//
// Created by dran on 4/9/22.
//

#ifndef NS_PA_2_CONSTANTS_H
#define NS_PA_2_CONSTANTS_H

#define FINISHED -1
#define SUCCESS 0
#define FAIL 1

#define TERMINATE 10  // should these be an enum?
#define ENQUEUE 11

#define CLIENT_WORKER_THREADS 10
#define CLIENT_JOB_STACK_SIZE 32

#define BLOCK_LIST_PATH "./blocklist"

#define CACHE_FILE_ROOT "./cache"
#define CACHE_HASH_LENGTH 33  // 32 bytes for md5sum output

#define KEEP_ALIVE_TIMEOUT 10

#define PRINTOUT_BUFFER_SIZE 512
#define JOB_REQUEST_BUFFER_SIZE 4096
#define MAX_URL_SIZE 2048

#endif //NS_PA_2_CONSTANTS_H
