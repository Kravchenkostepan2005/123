# Minimal Makefile to build a static lib for semantics
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
AR ?= ar
ARFLAGS ?= rcs

LIB = libsemantics.a
OBJS = semantics.o

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

semantics.o: semantics.c semantics.h
	$(CC) $(CFLAGS) -c semantics.c -o $@

clean:
	rm -f $(OBJS) $(LIB)
