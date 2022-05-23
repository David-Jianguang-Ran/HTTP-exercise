# Programming Assignment 3 - HTTP Proxy

HTTP proxy that supports caching and link prefetching.  
This proxy only support HTTP GET method.    
This program is an academic exercise.  

## what does it do?

run server executable with arguments: `<port#> <cached-file-ttl-seconds>`  
optionally support a `blocklist` file where blocked hosts are listed as `<domain>` or `<domain>:<port>` or `<ip>:<port>`  
This `blocklist` file needs to be in the working directory of the server executable.  

## Overview
![](./overview.png)


## what is in the makefile?

- do `make server` to compile server executable. 
The finished binary will be at ./executables/server the default cache root is ./executables/cache
- there are some non-automated unit tests, do `make tests` to compile them.  
- there are some small scale experiments I used to learn about the behaviour HTTP. do `make experiments` to compile them.

## few thing to note 

- most comments and documentations are in .h files  
- client request / response and prefetch uses pretty much the same code path and same objects for ease of development.  
- While a resource is being retrieved for the first time from server, before it is finished. 
The `cache_record_get_or_create` will return an unavailable status when another thread checks the same resource.
This means if a resource is requested in quick succession, the first few requests will trigger a server get. 
The rationale behind this is that coordinating reading incomplete cache file is risky 
and blocking subsequent requests untill the first one finishes is bad for responsiveness to interactive users. 
- While the cache files are stored on disk, the cache record used for retrieving 
and coordinating multiple reader / writer thread lives in memory and will be lost after server shutdown. 
This means cached responses do not persist past the lifetime of the server program.  
- During `cache_record_get_or_create` this program will user `popen` to execute `md5sum` in the terminal. 
This seems pretty dangerous but given the time constraint and the scope of the assignment it seems reasonable. 
This is not how I live my life normally. 


## want to see extra printout?

- files such as `thread-safe-job-stack.c` and `worker.c` have DEBUG flags that will enable more printouts. 

## Library / works cited

- `uthash.h` is a hash table `library` maintained by Troy D. Hanson see https://troydhanson.github.io/uthash/  
- `request.c` is an HTTP test client made by Prof.Schreuder. The server does not include any code from it.  