# Makefile for iol-hat project

CC = gcc
CFLAGS = -Wall -Wextra -I.

# Source files
SRCS = iol-hat.c demo.c

# Header files
HEADERS = iol-hat.h

# Output executable
TARGET = iolhat_demo

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
