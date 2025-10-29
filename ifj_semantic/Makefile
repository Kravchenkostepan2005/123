CC=gcc
CFLAGS=-std=c17 -Wall -Wextra -O2 -Iinclude

# Expect scanner.c, parser.c, ast.c to be available in src or parent paths.
# You can override SCANNER, PARSER, AST paths when invoking make.
SCANNER?=../scanner.c
PARSER?=../parser.c
AST?=../ast.c

OBJS=semantic.o

all: test_semantic_integration

semantic.o: src/semantic.c include/semantic.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Build integration test; requires your scanner/parser/ast sources
# Example usage:
#   make SCANNER=../path/scanner.c PARSER=../path/parser.c AST=../path/ast.c

test_semantic_integration: $(OBJS) tests/test_semantic_integration.c $(SCANNER) $(PARSER) $(AST)
	$(CC) $(CFLAGS) -o $@ tests/test_semantic_integration.c $(OBJS) $(SCANNER) $(PARSER) $(AST)

clean:
	rm -f $(OBJS) test_semantic_integration

.PHONY: all clean
