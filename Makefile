CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11 -g

OBJS=semantic.o symtable.o

all: tests

semantic.o: semantic.c semantic.h
	$(CC) $(CFLAGS) -c semantic.c

symtable.o: symtable.c semantic.h
	$(CC) $(CFLAGS) -c symtable.c

libsemantic.a: $(OBJS)
	ar rcs libsemantic.a $(OBJS)

tests: libsemantic.a tests.c semantic.h
	$(CC) $(CFLAGS) tests.c -L. -lsemantic -o tests

clean:
	rm -f *.o *.a tests
