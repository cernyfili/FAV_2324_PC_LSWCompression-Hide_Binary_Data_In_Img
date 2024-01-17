# Makefile for FAV_PC_SP_23_24_BinaryDataInImg project

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -std=c99

# Source files
SRC = src/main.c \
      src/compression.c \
      src/image.c \
      src/payload.c \
      src/utils/logger.c \
      src/utils/memory_management.c \
      src/utils/data_structures.c \
      src/utils/dictionary.c \
      src/utils/utils.c \
      src/utils/trie.c

# Object files
OBJ = $(SRC:.c=.o)

# Header files
INC = -Isrc -Isrc/utils

# Target executable
TARGET = stegim.exe

# Libraries
LIBS = -lpng -lz  # Include zlib library

# Compilation rule
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIBS)

# Rule for compiling source files to object files
%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Clean rule
clean:
	rm -f $(TARGET) $(OBJ)
