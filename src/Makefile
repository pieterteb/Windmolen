# Compiler and flags.
CC = gcc
CFLAGS = -fdiagnostics-color=always -Wall -Wextra -Werror -Wno-error=unused-result -Wpedantic -Wshadow -Wconversion -Wunused -Wnull-dereference -Wformat=2 -fstack-protector-all -D_FORTIFY_SOURCE=2 -O -fsanitize=address

# Project files.
SRC = main.c bitboard.c
OBJ = $(SRC:.c=.o)
TARGET = windmolen

# Default target.
all: $(TARGET)

# Link objects into final executables.
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source into object files.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup build artefacts
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET)
