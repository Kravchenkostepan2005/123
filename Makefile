CC := gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS ?=

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
BIN := snmp2otel

TESTS := $(wildcard tests/*.c)
TEST_BIN := tests/run_tests

.PHONY: all debug run test clean

all: $(BIN)

debug: CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g -O0
debug: clean $(BIN)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

run: $(BIN)
	./$(BIN) || true

# For tests, compile all sources except main.c
TEST_SRC := $(filter-out src/main.c,$(SRC))

test: debug $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(TESTS)
	$(CC) $(CFLAGS) $(TEST_SRC) $(TESTS) -o $@ $(LDFLAGS)

clean:
	rm -rf build $(BIN) $(TEST_BIN)

