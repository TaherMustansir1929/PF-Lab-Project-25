# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -g $(shell pkg-config --cflags gtk4)

# Linker flags (libraries)
LIBS = -lcjson -lcurl -luuid $(shell pkg-config --libs gtk4)

# Source files
SRCS = $(wildcard src/*.c) $(wildcard src/api/*.c)
# Header files
HDRS = -I./include
# Executable name
TARGET = main

# Default rule: build the program directly
all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(HDRS) $(LIBS)

# Run the program
run: all
	./$(TARGET)

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./$(TARGET)

# Clean up generated files
clean:
	rm -f $(TARGET)
