# Compiler and base flags.
CC = gcc
BASEFLAGS = -std=c11 -fdiagnostics-color=always -Wall -Wextra -Werror \
            -Wno-error=unused-result -Wpedantic -Wshadow -Wconversion \
            -Wunused -Wnull-dereference -Wformat=2 -fstack-protector-all

# Build type flags.
DEBUGFLAGS   = -g -O0 -fsanitize=address
RELEASEFLAGS = -O3 -DNDEBUG -D_FORTIFY_SOURCE=2

# Default: release build.
CFLAGS = $(BASEFLAGS) $(RELEASEFLAGS)

# Project files.
SRC    = main.c bitboard.c position.c
OBJ    = $(SRC:.c=.o)
ASM    = $(SRC:.c=.s)
TARGET = windmolen

# Default target.
all: release

# Debug build (asserts enabled).
.PHONY: debug
debug: CFLAGS = $(BASEFLAGS) $(DEBUGFLAGS)
debug: $(TARGET)

# Release build (asserts disabled).
.PHONY: release
release: CFLAGS = $(BASEFLAGS) $(RELEASEFLAGS)
release: $(TARGET)

# Link objects into final executable.
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
