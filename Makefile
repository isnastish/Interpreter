CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -g
SRC = src

SRCS = $(SRC)/main.c $(SRC)/common.h $(SRC)/string_guard.h \
       $(SRC)/lexer.h $(SRC)/lexer.c $(SRC)/ast.h $(SRC)/ast.c \
       $(SRC)/parser.h $(SRC)/parser.c $(SRC)/eval.h $(SRC)/eval.c \
       $(SRC)/table.c $(SRC)/io.c $(SRC)/test.c

all: interpreter

interpreter: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRC)/main.c

clean:
	rm -f interpreter

run: interpreter
	./interpreter

.PHONY: all clean run
