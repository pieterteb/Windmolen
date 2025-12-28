# Compiler and flags
CC      := gcc
BASE    := -std=c23 -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion \
           -Wunused -Wnull-dereference -Wformat=2 \
           -fdiagnostics-color=always -Wno-error=unused-result
DEBUG   := -g -O0 -fsanitize=address -fstack-protector-all
RELEASE := -O3 -flto -DNDEBUG -fno-stack-protector -march=native

# Sources, objects, target
SRC     := main.c bitboard.c board.c engine.c evaluation.c move_generation.c move_picker.c options.c position.c score.c search.c thread.c time_manager.c uci.c util.c zobrist.c
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
