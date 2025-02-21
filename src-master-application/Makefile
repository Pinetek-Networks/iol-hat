# Compiler and flags
CXX = g++
CC = gcc

# Common flags
COMMON_FLAGS = -Wall -Wextra \
               -I./include \
               -I./include/ilink \
               -I./include/osal \
               -I./include/sys \
               -I./ilink

# Debug flags
DEBUG_FLAGS = -g -O0 -DPINEDEBUG

# Release flags
RELEASE_FLAGS = -O2

# Choose build type based on input (default is release)
ifeq ($(BUILD),debug)
    BUILD_TYPE = debug
    CXXFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS) -std=c++17
    CFLAGS = $(COMMON_FLAGS) $(DEBUG_FLAGS)
    BUILD_DIR = build/debug
    BIN_DIR = bin/debug
else
    BUILD_TYPE = release
    CXXFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS) -std=c++17
    CFLAGS = $(COMMON_FLAGS) $(RELEASE_FLAGS)
    BUILD_DIR = build/release
    BIN_DIR = bin/release
endif

# Directories
SRC_DIR = src
ILINK_DIR = ilink
IOL_OSAL_DIR = iol_osal
LIB_DIR = lib

# Source files
SRCS_CXX = $(wildcard $(SRC_DIR)/*.cc)
SRCS_C_ILINK = $(wildcard $(ILINK_DIR)/*.c)
SRCS_C_OSAL = $(wildcard $(IOL_OSAL_DIR)/*.c)

# Object files
OBJS_CXX = $(SRCS_CXX:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
OBJS_C_ILINK = $(SRCS_C_ILINK:$(ILINK_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS_C_OSAL = $(SRCS_C_OSAL:$(IOL_OSAL_DIR)/%.c=$(BUILD_DIR)/%.o)

# Output binary
TARGET = $(BIN_DIR)/iol_master_app

# External libraries
LIBS = -L$(LIB_DIR) -lgpiod -lpthread -lrt

# Default target
all: info directories $(TARGET)

# Print build type info
info:
	@echo "Building in $(BUILD_TYPE) mode"

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Link the final executable
$(TARGET): $(OBJS_CXX) $(OBJS_C_ILINK) $(OBJS_C_OSAL)
	$(CXX) $^ -o $@ $(LIBS)
	@echo "Build complete: $@"

# Compile C++ source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C source files from ilink directory
$(BUILD_DIR)/%.o: $(ILINK_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C source files from iol_osal directory
$(BUILD_DIR)/%.o: $(IOL_OSAL_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf build bin

# Clean and rebuild
rebuild: clean all

# Debug target
debug:
	$(MAKE) BUILD=debug

# Release target
release:
	$(MAKE) BUILD=release

# Install target (adjust install path as needed)
install: all
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin

# Generate dependencies
depend: $(SRCS_CXX) $(SRCS_C_ILINK) $(SRCS_C_OSAL)
	$(CXX) $(CXXFLAGS) -MM $(SRCS_CXX) > .depend
	$(CC) $(CFLAGS) -MM $(SRCS_C_ILINK) >> .depend
	$(CC) $(CFLAGS) -MM $(SRCS_C_OSAL) >> .depend

.PHONY: all clean rebuild debug release install directories depend info

# Include dependencies if they exist
-include .depend

