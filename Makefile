SRC = src
CFLAGS = -Wall -Werror

SCANNER  = $(SRC)/scanner.c
PARSER   = $(SRC)/parser.c
TYPE_CHK = $(SRC)/type_check.c
CODE_GEN = $(SRC)/code_generation.c
COMPILER = $(SRC)/compiler.c

SOURCES = $(SCANNER) \
		  $(PARSER) \
		  $(TYPE_CHK) \
		  $(CODE_GEN) \
		  $(COMPILER)

BINARY = compile

$(BINARY): $(SOURCES)
	@$(CC) $(CFLAGS) $(SOURCES) -o $@

