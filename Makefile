CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -I./include
LDFLAGS =

SRC = src/symtable.c
TEST = tests/test_symtable.c
OBJ = $(SRC:.c=.o)

all: test

build:
	$(CC) $(CFLAGS) -c src/symtable.c -o src/symtable.o

test: build
	$(CC) $(CFLAGS) $(LDFLAGS) src/symtable.o $(TEST) -o symtable_tests
	./symtable_tests | cat

clean:
	rm -f src/*.o symtable_tests

.PHONY: all build test clean

