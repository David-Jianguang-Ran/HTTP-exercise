# PA2 - HTTP Server


## what is the in make file?

- do `make server` to compile server executable. 
The finished binary will be in ./executables the default document root is in ./executables/www
- there are some non-automated unit tests, do `make tests` to compile them.  
- there are some small scale experiments I used to learn about the behaviour HTTP. do `make tests` to compile them.

## want to see extra printout?

- files such as `thread-safe-job-stack.c` and `worker.c` have DEBUG flags that will enable more printouts. 