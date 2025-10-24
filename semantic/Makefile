CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -Werror -Iinclude
LDFLAGS=

SRC=src/symtable.c src/semantic.c
OBJ=$(SRC:.c=.o)

all: test

libsemantic.a: $(OBJ)
	ar rcs $@ $^

build-tests: libsemantic.a tests/test_semantic.c
	$(CC) $(CFLAGS) -o build/test_semantic tests/test_semantic.c libsemantic.a $(LDFLAGS)

clean:
	rm -rf build libsemantic.a $(OBJ)

mkdirs:
	mkdir -p build

.PHONY: all clean mkdirs build-tests test

test: mkdirs build-tests
	./build/test_semantic
