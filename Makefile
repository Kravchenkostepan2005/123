CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2 -g -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE
LDFLAGS :=

SRC_DIR := src
INC_DIR := include
THIRD_DIR := third_party
TEST_DIR := tests

SRCS := \
  $(SRC_DIR)/snmp2otel.c \
  $(SRC_DIR)/ber.c \
  $(SRC_DIR)/oid.c \
  $(SRC_DIR)/snmp.c \
  $(SRC_DIR)/otlp.c \
  $(SRC_DIR)/mapping.c

OBJS := $(SRCS:.c=.o)

TARGET := snmp2otel

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(THIRD_DIR) -o $@ $(OBJS) $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(THIRD_DIR) -c $< -o $@

$(SRC_DIR)/snmp2otel.o: $(SRC_DIR)/snmp2otel.c $(INC_DIR)/ber.h $(INC_DIR)/oid.h $(INC_DIR)/snmp.h $(INC_DIR)/otlp.h $(INC_DIR)/mapping.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(THIRD_DIR) -c $< -o $@

run: $(TARGET)
	./$(TARGET) -t 127.0.0.1 -o oids.txt -e http://localhost:4318/v1/metrics -v || true

TEST_SRCS := \
  $(TEST_DIR)/test_runner.c \
  $(TEST_DIR)/test_oid.c \
  $(TEST_DIR)/test_ber.c \
  $(TEST_DIR)/test_url.c \
  $(SRC_DIR)/ber.c \
  $(SRC_DIR)/oid.c \
  $(SRC_DIR)/otlp.c

TEST_OBJS := $(TEST_SRCS:.c=.o)

test: $(TEST_OBJS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(THIRD_DIR) -o $(TEST_DIR)/tests $(TEST_OBJS) $(LDFLAGS)
	$(TEST_DIR)/tests

$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(THIRD_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_DIR)/tests

