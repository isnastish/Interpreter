CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -g

all: interpreter

interpreter: main.c common.h string_guard.h lexer.h lexer.c ast.h ast.c parser.h parser.c table.c io.c test.c
	$(CC) $(CFLAGS) -o $@ main.c

clean:
	rm -f interpreter

run: interpreter
	./interpreter

.PHONY: all clean run
