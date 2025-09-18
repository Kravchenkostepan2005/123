APP := snmp2otel
BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS := 
LDLIBS := -pthread

CORE_SRCS := \
    $(SRC_DIR)/asn1.cpp \
    $(SRC_DIR)/oid.cpp \
    $(SRC_DIR)/snmp.cpp \
    $(SRC_DIR)/http.cpp \
    $(SRC_DIR)/otel.cpp \
    $(SRC_DIR)/util.cpp

SRCS := $(CORE_SRCS) $(SRC_DIR)/main.cpp

CORE_OBJS := $(CORE_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS := $(CORE_OBJS) $(BUILD_DIR)/main.o

TEST_SRCS := $(TEST_DIR)/test_main.cpp
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/tests/%.o)
TEST_BIN := $(BUILD_DIR)/tests/tests

.PHONY: all run test clean

all: $(APP)

$(APP): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c -o $@ $<

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(TEST_DIR) -c -o $@ $<

$(TEST_BIN): $(TEST_OBJS) $(CORE_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

run: $(APP)
	./$(APP) -h | cat

test: $(TEST_BIN)
	$(TEST_BIN) | cat

clean:
	rm -rf $(BUILD_DIR) $(APP)

