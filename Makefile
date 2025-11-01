# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -g $(shell pkg-config --cflags gtk+-3.0)

# Linker flags (libraries)
# For dynamic linking (default):
LIBS = -lcjson -lcurl -luuid $(shell pkg-config --libs gtk+-3.0) -lsqlite3

# For static linking (portable executable):
# Uncomment the following lines and comment out the dynamic LIBS above
# LIBS = -static -lcjson -lcurl -luuid $(shell pkg-config --libs --static gtk+-3.0) -lsqlite3 -lpthread -lws2_32 -lcrypt32

# Source files
SRCS = $(wildcard src/*.c) $(wildcard src/api/*.c) $(wildcard src/db/*.c)
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

# Create distribution package with bundled DLLs
dist: all
	@echo "Creating distribution package..."
	@bash bundle-dlls.sh

# Create single-file portable ZIP (easier to distribute)
single-zip: dist
	@echo "Creating single ZIP file..."
	@powershell -ExecutionPolicy Bypass -File create-single-exe.ps1

# Create self-extracting installer (requires 7-Zip)
installer: dist
	@echo "Creating self-extracting installer..."
	@powershell -ExecutionPolicy Bypass -File create-installer.ps1

# Help message
help:
	@echo "Available targets:"
	@echo "  make          - Build the application (dynamic linking)"
	@echo "  make run      - Build and run the application"
	@echo "  make dist     - Create dist folder with all DLLs"
	@echo "  make single-zip - Create portable ZIP file (recommended)"
	@echo "  make installer - Create self-extracting .exe (needs 7-Zip)"
	@echo "  make clean    - Remove build artifacts"

# Clean up generated files
clean:
	rm -f $(TARGET)
	rm -rf dist/
	rm -f *.zip *.exe temp-archive.7z sfx-config.txt
