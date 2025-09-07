# Compiler and base flags.
CC = gcc

AVX512FLAGS = -mavx512f -mavx512dq

BASEFLAGS = -std=c11 -fdiagnostics-color=always -Wall -Wextra -Werror \
            -Wno-error=unused-result -Wpedantic -Wshadow -Wconversion \
            -Wunused -Wnull-dereference -Wformat=2 -fstack-protector-all \
			$(AVX512FLAGS)


# Build type flags.
DEBUGFLAGS   = -g -O0 -fsanitize=address
RELEASEFLAGS = -O3 -DNDEBUG -D_FORTIFY_SOURCE=2

# Additional include directories (space-separated, each prefixed with -I)
INCLUDES = -I../../src

# Default: release build.
CFLAGS = $(BASEFLAGS) $(RELEASEFLAGS) $(INCLUDES)

# Project files.
SRC    = main.c ../../src/bitboard.c ../../src/util.c
OBJ    = $(SRC:.c=.o)
TARGET = magic_finder

# Default target.
all: release

# Debug build (asserts enabled).
.PHONY: debug
debug: CFLAGS = $(BASEFLAGS) $(DEBUGFLAGS) $(INCLUDES)
debug: $(TARGET)

# Release build (asserts disabled).
.PHONY: release
release: CFLAGS = $(BASEFLAGS) $(RELEASEFLAGS) $(INCLUDES)
release: $(TARGET)

# Link objects into final executable.
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source into object files.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup build artefacts.
.PHONY: clean
clean:
	rm -f $(ASM) $(OBJ) $(TARGET)

# Run program.
.PHONY: run
run: $(TARGET)
	./$(TARGET)
