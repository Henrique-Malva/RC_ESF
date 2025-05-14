# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Ilib

# Directories
SRC_DIR := src
INC_DIR := inc
LIB_DIR := lib
BUILD_DIR := build
BIN_DIR := bin

# Find all C source files in lib and src
SRC_FILES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)

# Object files will be placed in the build directory
OBJ_FILES := $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRC_FILES)))

# Output executable
TARGET := $(BIN_DIR)/main

# Default rule
all: $(TARGET)

# Linking the final executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each .c to .o in the build directory
$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

# Clean rule
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
