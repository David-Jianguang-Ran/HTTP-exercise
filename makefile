modules = thread-safe-job-stack.c thread-safe-file.c job.c

server: directory-structure
	gcc -Wall -pthread -o ./executables/server server.c worker.c $(modules)

directory-structure:
	test -d "./executables" || mkdir "./executables"
	test -d "./executables/www" || mkdir "./executables/www"

experiments: directory-structure
	echo "no experiments at the moment"

tests: directory-structure
	gcc -Wextra -Wall -pthread -o ./executables/block-table-test block-table.c block-table-test.c
	#gcc -Wall -o ./executables/parsing-test parsing-test.c worker.c $(modules)
	#gcc -Wextra -Wall -pthread -o ./executables/job-stack-test job-stack-test.c $(modules)