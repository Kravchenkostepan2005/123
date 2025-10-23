CC = gcc
# Ensure POSIX/GNU extensions like strdup are declared
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -Iinclude -g

SRC = src/semantic.c src/symtable.c src/ast.c
TEST_SRC = tests/test_semantic.c
BIN = semantic_tests

all: test

$(BIN): $(SRC) $(TEST_SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(TEST_SRC)

.PHONY: test clean

test: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) *.o src/*.o
