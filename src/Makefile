# Compiler and flags.
CC = gcc
CFLAGS = -fdiagnostics-color=always -Wall -Wextra -Werror -Wno-error=unused-result -Wpedantic -Wshadow -Wconversion -Wunused -Wnull-dereference -Wformat=2 -fstack-protector-all -D_FORTIFY_SOURCE=2 -O3 -fsanitize=address

# Project files.
SRC = main.c bitboard.c position.c
OBJ = $(SRC:.c=.o)
ASM = $(SRC:.c=.s)
TARGET = windmolen

# Default target.
all: $(TARGET)

# Link objects into final executables.
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source into object files.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile specified file into object file.
.PHONY: build
build:
	$(CC) $(CFLAGS) -c $(FILE) -o $(FILE:.c=.o)

# Compile specified file into assembly.
.PHONY: asm
asm:
	$(CC) $(CFLAGS) -S $(FILE) -o $(FILE:.c=.s)

# Cleanup build artefacts.
.PHONY: clean
clean:
	rm -f $(ASM) $(OBJ) $(TARGET)

# Run program.
.PHONY: run
run: $(TARGET)
	./$(TARGET)
