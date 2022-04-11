modules = thread-safe-job-stack.c thread-safe-file.c job.c block-table.c cache-record.c parsing.c worker.c

server: directory-structure
	gcc -Wall -pthread -o ./executables/server server.c $(modules)

directory-structure:
	test -d "./executables" || mkdir "./executables"
	test -d "./executables/cache" || mkdir "./executables/cache"

experiments: directory-structure
	gcc -Wall -o ./executables/simple-client-experiment simple-client-experiment.c
	gcc -Wall -o ./executables/test-client-experiment test-client-experiment.c
	gcc -Wall -o ./executables/slow-loris-client slow-client.c

tests: directory-structure
	gcc -Wall -pthread -o ./executables/block-table-test block-table.c block-table-test.c
	gcc -Wall -pthread -o ./executables/cache-record-test cache-record-test.c cache-record.c
	gcc -Wall -o ./executables/parsing-test parsing-test.c parsing.c
	gcc -Wall -pthread -o ./executables/resolve-host-test resolve-host-test.c $(modules)