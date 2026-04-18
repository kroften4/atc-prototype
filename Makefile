CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -pthread -std=c23
CPPFLAGS := -MMD -MP -Iinclude
LDFLAGS ?=
LDLIBS := -lncurses -lm

DEBUG ?= 0
ifeq ($(DEBUG),1)
	CFLAGS += -ggdb -O0 -DDEBUG
else
	CFLAGS += -O2
endif

BIN_DIR ?= bin
BUILD_DIR := build

SRC_COMMON := $(shell find src/common/ -type f -name '*.c')
SRC_CLIENT := $(shell find src/client/ -type f -name '*.c')
SRC_SERVER := $(shell find src/server/ -type f -name '*.c')
OBJ_COMMON := $(SRC_COMMON:%.c=$(BUILD_DIR)/%.o)
OBJ_CLIENT := $(SRC_CLIENT:%.c=$(BUILD_DIR)/%.o)
OBJ_SERVER := $(SRC_SERVER:%.c=$(BUILD_DIR)/%.o)
OBJ_ALL := $(OBJ_COMMON) $(OBJ_CLIENT) $(OBJ_SERVER)
OBJ_EXCLUDED_MAIN := $(OBJ_COMMON) $(filter-out %/main.o, $(OBJ_CLIENT) $(OBJ_SERVER))

UNITY_CONFIG := test/unity-config.yaml
UNITY_ROOT := lib/unity
UNITY_SRC := $(wildcard $(UNITY_ROOT)/src/*.c)
UNITY_OBJ := $(UNITY_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_SRC := $(shell find test/ -type f -name '*.c')
TEST_OBJ := $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_RUNNERS_SRC := $(TEST_SRC:test/%.c=$(BUILD_DIR)/test_runners/%_Runner.c)
TEST_BINS := $(TEST_RUNNERS_SRC:$(BUILD_DIR)/test_runners/%_Runner.c=$(BIN_DIR)/test/%)
TEST_CPPFLAGS := -I$(UNITY_ROOT)/src/ -DUNITY_SUPPORT_TEST_CASES

.PHONY: all build build-test build-client build-server test clean
.DEFAULT_GOAL: all
.SECONDARY: $(TEST_RUNNERS_SRC) $(TEST_RUNNERS_SRC:.c=.o) $(TEST_OBJ)

all: build

build: build-client build-server build-test

-include $(OBJ_ALL:.o=.d)
-include $(UNITY_OBJ:.o=.d)
-include $(TEST_OBJ:.o=.d)
-include $(TEST_RUNNERS_SRC:.c=.d)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BIN_DIR)/client: $(OBJ_COMMON) $(OBJ_CLIENT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/server: $(OBJ_COMMON) $(OBJ_SERVER)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

build-client: $(BIN_DIR)/client

build-server: $(BIN_DIR)/server

# TESTS
$(BUILD_DIR)/test_runners/%.o: $(BUILD_DIR)/test_runners/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TEST_CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/test/%.o: test/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TEST_CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/test_runners/%_Runner.c: test/%.c
	@mkdir -p $(@D)
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb $< $@ $(UNITY_CONFIG)

$(BIN_DIR)/test/%: $(BUILD_DIR)/test_runners/%_Runner.o $(BUILD_DIR)/test/%.o $(UNITY_OBJ) $(OBJ_EXCLUDED_MAIN)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

build-test: $(TEST_BINS)

test: build-test
	@echo "> Test sources: $(TEST_SRC)"
	@scripts/run_test_bins.sh $(TEST_BINS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
