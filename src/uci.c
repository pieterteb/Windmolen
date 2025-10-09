#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "uci.h"

#include "move_generation.h"
#include "position.h"
#include "types.h"


#define LINE_BUFFER_SIZE   4096
#define MAX_COMMAND_LENGTH 1024



struct Position main_position;


static void trim_newline(char* string) {
    assert(string != NULL);

    size_t length = strlen(string);

    if (length != 0) string[length - 1] = '\0';
}

static const char* next_argument(const char* argument_string, char argument[MAX_COMMAND_LENGTH]) {
    assert(argument != NULL);

    if (argument_string == NULL || argument_string[0] == '\0') {
        argument[0] = '\0';

        return NULL;
    }

    while (isblank(*argument_string))
        ++argument_string;
    const char* argument_start = argument_string;

    while (!isblank(*argument_string) && *argument_string != '\0')
        ++argument_string;
    size_t argument_length = (size_t)(argument_string - argument_start);

    memcpy(argument, argument_start, argument_length);
    argument[argument_length] = '\0';

    return argument_string;
}

static Move parse_move(const char* move_string) {
    assert(move_string != NULL);
    assert(move_string[4] == '\0' || move_string[5] == '\0');

    static const MoveType char_to_promotion[] = {['n'] = MOVE_TYPE_KNIGHT_PROMOTION,
                                                 ['b'] = MOVE_TYPE_BISHOP_PROMOTION,
                                                 ['r'] = MOVE_TYPE_ROOK_PROMOTION,
                                                 ['q'] = MOVE_TYPE_ROOK_PROMOTION};

    Square source      = coordinate_square(char_to_file(move_string[0]), char_to_rank(move_string[1]));
    Square destination = coordinate_square(char_to_file(move_string[2]), char_to_rank(move_string[3]));
    MoveType move_type = (MoveType)((move_string[4] == '\0') ? MOVE_TYPE_NORMAL
                                                             : char_to_promotion[(int)move_string[5]]);

    return new_move(source, destination, move_type);
}


static void uci_id() {
    puts("id name Windmolen");
    puts("id author Pieter te Brake");
}

static void uci_options() {
    // puts("");
}

static void handle_position(const char* argument_string) {
    char argument[MAX_COMMAND_LENGTH];
    argument_string = next_argument(argument_string, argument);

    if (strcmp(argument, "fen") == 0)
        argument_string = position_from_FEN(&main_position, argument_string);
    else if (strcmp(argument, "startpos") == 0)
        position_from_startpos(&main_position);

    argument_string = next_argument(argument_string, argument);

    if (strcmp(argument, "moves") == 0) {
        argument_string = next_argument(argument_string, argument);

        while (argument_string != NULL) {
            Move move = parse_move(argument);
            do_move(&main_position, move);

            argument_string = next_argument(argument_string, argument);
        }
    }
}

// static void handle_go(const char* argument_string) {

// }


void uci_loop() {
    // Minimum command support:
    //
    // uci
    // isready
    // position
    // go
    // stop
    // quit

    const size_t supported_command_count     = 6;
    static const char* supported_commands[6] = {"uci", "isready", "position", "go", "stop", "quit"};

    char line[LINE_BUFFER_SIZE];

    while (fgets(line, LINE_BUFFER_SIZE, stdin) != NULL) {
        trim_newline(line);
        if (line[0] == '\0') continue;

        char command[MAX_COMMAND_LENGTH] = {0};
        const char* arguments_string     = NULL;

        char* parse_ptr = line;
        while (*parse_ptr != '\0') {
            /* Skip whitespace. */
            while (isblank(*parse_ptr))
                ++parse_ptr;
            if (*parse_ptr == '\0') break;

            char* token_start = parse_ptr;
            while (!isblank(*parse_ptr) && *parse_ptr != '\0')
                ++parse_ptr;
            size_t token_length = (size_t)(parse_ptr - token_start);

            /* Check whether this is a known command. */
            for (size_t i = 0; i < supported_command_count; ++i) {
                size_t command_length = strlen(supported_commands[i]);
                if (token_length == command_length
                    && strncmp(token_start, supported_commands[i], command_length) == 0) {
                    memcpy(command, token_start, token_length);
                    command[token_length] = '\0';

                    while (isspace(*parse_ptr))
                        ++parse_ptr;
                    arguments_string = parse_ptr;

                    goto found_command;
                }
            }
        }

found_command:
        if (command[0] == '\0') continue;

        if (strcmp(command, "uci") == 0) {
            uci_id();
            uci_options();

            puts("uciok");
            fflush(stdout);
        } else if (strcmp(command, "isready") == 0) {
            puts("readyok");
            fflush(stdout);
        } else if (strcmp(command, "position") == 0) {
            handle_position(arguments_string);
        } else if (strcmp(command, "go") == 0) {
            // handle_go(arguments_string);
        } else if (strcmp(command, "stop") == 0) {
            // stop_search();
        } else if (strcmp(command, "quit") == 0) {
            // stop_search();

            break;
        }
    }
}
