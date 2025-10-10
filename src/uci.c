#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "uci.h"

#include "move_generation.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "types.h"



#define LINE_BUFFER_SIZE 4096
#define MAX_TOKEN_LENGTH 1024


struct Position main_position;
struct SearchState main_search_state;



static void trim_newline(char* string) {
    assert(string != NULL);

    size_t length = strlen(string);

    if (length != 0) string[length - 1] = '\0';
}

/* Returns pointer to first character after the found token or `NULL` if no token was found. The found token
 * gets copied to `token`. If no token was found, `token` is an empty string. */
static const char* next_token(const char* token_string, char token[MAX_TOKEN_LENGTH]) {
    assert(token != NULL);

    if (token_string == NULL) {
        token[0] = '\0';
        return NULL;
    }

    while (isblank(*token_string))
        ++token_string;
    const char* token_start = token_string;

    while (!isblank(*token_string) && *token_string != '\0')
        ++token_string;
    size_t token_length = (size_t)(token_string - token_start);

    memcpy(token, token_start, token_length);
    token[token_length] = '\0';

    return (token_length == 0) ? NULL : token_string;
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
    puts("id name Windmolen\n" "id author Pieter te Brake");
}

static void uci_options() {
    // puts("");
}

static void handle_position(const char* argument_string) {
    char argument[MAX_TOKEN_LENGTH];
    argument_string = next_token(argument_string, argument);

    if (strcmp(argument, "fen") == 0)
        argument_string = position_from_FEN(&main_position, argument_string);
    else if (strcmp(argument, "startpos") == 0)
        position_from_startpos(&main_position);

    argument_string = next_token(argument_string, argument);

    if (strcmp(argument, "moves") == 0) {
        argument_string = next_token(argument_string, argument);

        while (argument_string != NULL) {
            Move move = parse_move(argument);
            do_move(&main_position, move);

            argument_string = next_token(argument_string, argument);
        }
    }
}

static void handle_go(const char* argument_string) {
    assert(argument_string != NULL);

    // Supported go commands:
    //
    // searchmoves
    // infinite

    char argument[MAX_TOKEN_LENGTH];
    argument_string = next_token(argument_string, argument);

    main_search_state.move_count = 0;

    while (argument_string != NULL) {
        if (strcmp(argument, "searchmoves") == 0) {
            argument_string = next_token(argument_string, argument);

            while (argument_string != NULL) {
                main_search_state.movelist[main_search_state.move_count++] = parse_move(argument);
                print_move(stdout, main_search_state.movelist[main_search_state.move_count - 1]);

                argument_string = next_token(argument_string, argument);
            }
        } else if (strcmp(argument, "infinite") == 0) {
        }

        argument_string = next_token(argument_string, argument);
    }

    start_search_thread(&main_search_state);
}


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

        char command[MAX_TOKEN_LENGTH] = {0};
        const char* token_string = line;

        do {
            token_string = next_token(token_string, command);

            /* Check whether this is a known command. */
            for (size_t i = 0; i < supported_command_count; ++i)
                if (strcmp(command, supported_commands[i]) == 0) goto found_command;
        } while (token_string != NULL);

found_command:
        if (token_string == NULL) continue;

        if (strcmp(command, "uci") == 0) {
            uci_id();
            uci_options();

            puts("uciok");
            fflush(stdout);
        } else if (strcmp(command, "isready") == 0) {
            puts("readyok");
            fflush(stdout);
        } else if (strcmp(command, "position") == 0) {
            handle_position(token_string);
        } else if (strcmp(command, "go") == 0) {
            handle_go(token_string);
        } else if (strcmp(command, "stop") == 0) {
            if (main_search_state.is_searching) {
                Move best_move = stop_search_thread(&main_search_state);
                printf("bestmove ");
                print_move(stdout, best_move);
                putc('\n', stdout);
                fflush(stdout);

                do_move(&main_position, best_move);
            }
        } else if (strcmp(command, "quit") == 0) {
            if (main_search_state.is_searching) {
                Move best_move = stop_search_thread(&main_search_state);
                printf("bestmove ");
                print_move(stdout, best_move);
                putc('\n', stdout);
                fflush(stdout);

                do_move(&main_position, best_move);
            }

            break;
        }
    }
}
