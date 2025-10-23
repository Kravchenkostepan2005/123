CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2

SRCS=semantic.c symtable.c test_semantic.c
OBJS=$(SRCS:.c=.o)

all: test

%.o: %.c
	$(CC) $(CFLAGS) -I. -c $< -o $@

test: $(OBJS)
	$(CC) $(CFLAGS) -I. $(OBJS) -o semantic_tests

run: test
	./semantic_tests

clean:
	rm -f $(OBJS) semantic_tests
