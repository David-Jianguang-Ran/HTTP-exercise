modules = thread-safe-job-stack.c thread-safe-file.c job.c block-table.c

server: directory-structure
	gcc -Wall -pthread -o ./executables/server server.c worker.c $(modules)

directory-structure:
	test -d "./executables" || mkdir "./executables"
	test -d "./executables/www" || mkdir "./executables/www"

experiments: directory-structure
	gcc -Wall -o ./executables/resolve-host-experiment resolve-host-experiment.c
	gcc -Wall -o ./executables/simple-client-experiment simple-client-experiment.c

tests: directory-structure
	gcc -Wextra -Wall -pthread -o ./executables/block-table-test block-table.c block-table-test.c
	gcc -Wall -o ./executables/parsing-test parsing-test.c parsing.c
	#gcc -Wextra -Wall -pthread -o ./executables/job-stack-test job-stack-test.c $(modules)