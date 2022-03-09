directory-structure:
	test -d "./executables" || mkdir "./executables"
	test -d "./executables/www" || mkdir "./executables/www"

experiments: directory-structure
	gcc -Wall -o ./executables/new-line-test new-line-char-test.c
	gcc -Wall -o ./executables/closed-socket-test closed-socket-test.c
	gcc -Wall -o ./executables/worker-test worker-test.c