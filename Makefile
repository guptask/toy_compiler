COMPILE:
	@echo Compiling using the following command:
	gcc -Wall -Werror scanner.c parser.c type_check.c compiler.c -o compile

