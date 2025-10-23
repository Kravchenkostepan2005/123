CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2

SRC = types.c symtable.c analyzer.c demo_adapter.c
OBJ = $(SRC:.c=.o)
INC = -I.

.PHONY: all clean demo

all: libsemantic.a demo

libsemantic.a: $(OBJ)
	ar rcs $@ $^

demo: demo.o libsemantic.a
	$(CC) $(CFLAGS) -o $@ $^ $(INC)

demo.o: demo.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INC)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INC)

clean:
	rm -f $(OBJ) demo.o libsemantic.a demo
