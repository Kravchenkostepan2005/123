APP_NAME := snmp2otel
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS :=

SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
TEST_DIR := tests

SRCS_ALL := $(wildcard $(SRC_DIR)/*.cpp)
LIB_SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(SRCS_ALL))
LIB_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))
APP_OBJ := $(BUILD_DIR)/main.o

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SRCS))

.PHONY: all run test clean dirs

all: dirs $(BIN_DIR)/$(APP_NAME)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(BUILD_DIR)/*.d

$(BIN_DIR)/$(APP_NAME): $(LIB_OBJS) $(APP_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: all
	@echo "Running $(APP_NAME)"
	@$(BIN_DIR)/$(APP_NAME) $(ARGS)

$(BIN_DIR)/%: $(TEST_DIR)/%.cpp $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJS) -o $@ $(LDFLAGS)

test: all $(TEST_BINS)
	@set -e; \
	for t in $(TEST_BINS); do \
		echo "Running $$t"; \
		$$t || exit 1; \
	done; \
	echo "All tests passed"

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

