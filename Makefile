CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -pthread -std=c23 -MMD -MP
CFLAGS += -Iinclude
LDLIBS := -lncurses -lm

BIN_DIR := bin
BUILD_DIR := build

DEBUG ?= 0
ifeq ($(DEBUG),1)
	CFLAGS += -ggdb -O0 -DDEBUG
else
	CFLAGS += -O2
endif

SRC_COMMON := $(wildcard src/common/*.c)
SRC_CLIENT := $(wildcard src/client/*.c)
SRC_SERVER := $(wildcard src/server/*.c)
OBJ_COMMON := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC_COMMON))
OBJ_CLIENT := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC_CLIENT))
OBJ_SERVER := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC_SERVER))
OBJ_ALL := $(OBJ_COMMON) $(OBJ_CLIENT) $(OBJ_SERVER)

.PHONY: all clean

all: $(BIN_DIR)/server $(BIN_DIR)/client

-include $(OBJ_ALL:%.o=%.d)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/client: $(OBJ_COMMON) $(OBJ_CLIENT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_DIR)/server: $(OBJ_COMMON) $(OBJ_SERVER)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -rf $(BUILD_DIR)/
	rm -rf $(BIN_DIR)/
