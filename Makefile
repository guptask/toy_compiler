COMPILE:
	@echo Compiling using the following command:
	gcc -Wall -Werror src/scanner.c src/parser.c src/type_check.c src/code_generation.c src/compiler.c -o compile

