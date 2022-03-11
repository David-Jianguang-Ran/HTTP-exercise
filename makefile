directory-structure:
	test -d "./executables" || mkdir "./executables"
	test -d "./executables/www" || mkdir "./executables/www"

experiments: directory-structure
	gcc -Wall -o ./executables/new-line-experiment new-line-char-experiment.c
	gcc -Wall -o ./executables/closed-socket-experiment closed-socket-experiment.c

tests: directory-structure
	gcc -Wall -o ./executables/parsing-test parsing-test.c
	gcc -Wall -o ./executables/send-recv-test send-recv-test.c