CC= gcc
CFLAGS= -c -Wall -Werror -I$(SRC)
LDFLAGS=
SRC= src
SOURCES= $(wildcard $(SRC)/*.c)
INCLUDIR= $(wildcard $(SRC)/*.h)
OBJECTS= $(join $(addsuffix ../, $(dir $(SOURCES))), $(notdir $(SOURCES:.c=.o)))

EXECUTABLE = compile

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: $(SRC)/%.c $(INCLUDIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) *.o

.PHONY: all clean
