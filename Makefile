CC=gcc
CFLAGS=-std=c17 -Wall -Wextra -Werror -O2
LDFLAGS=

SRCS=ast.c semantics.c
OBJS=$(SRCS:.c=.o)
LIB=libifjsem.a

all: $(LIB) example

$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

example: example.o $(LIB)
	$(CC) $(CFLAGS) -o $@ example.o $(LIB) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(LIB) example example.o

.PHONY: all clean
