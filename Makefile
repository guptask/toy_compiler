COMPILE:
	@echo Compiling using the following command:
	gcc -Wall -Werror source/scanner.c source/parser.c source/type_check.c source/code_generation.c source/compiler.c -o compile

