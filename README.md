# PA2 - HTTP Server


## what is in the makefile?

- do `make server` to compile server executable. 
The finished binary will be at ./executables/server the default document root is ./executables/www
- there are some non-automated unit tests, do `make tests` to compile them.  
- there are some small scale experiments I used to learn about the behaviour HTTP. do `make tests` to compile them.

## want to see extra printout?

- files such as `thread-safe-job-stack.c` and `worker.c` have DEBUG flags that will enable more printouts. 