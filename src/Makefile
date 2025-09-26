# Compiler and flags
CC      := gcc
BASE    := -std=c11 -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion \
           -Wunused -Wnull-dereference -Wformat=2 -fstack-protector-all \
           -fdiagnostics-color=always -Wno-error=unused-result
DEBUG   := -g -O0 -fsanitize=address
RELEASE := -O3 -flto -DNDEBUG -D_FORTIFY_SOURCE=2

# Sources, objects, target
SRC     := main.c bitboard.c move_generation.c position.c
OBJ     := $(SRC:.c=.o)
TARGET  := windmolen

# Default build: release
CFLAGS  := $(BASE) $(RELEASE)

# Targets
.PHONY: all debug release clean run

all: release

debug: CFLAGS := $(BASE) $(DEBUG)
debug: clean $(TARGET)

release: CFLAGS := $(BASE) $(RELEASE)
release: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)
