#include "uci.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "move_generation.h"
#include "perft.h"
#include "position.h"
#include "time_manager.h"
#include "types.h"



#define LINE_BUFFER_SIZE 65536


const char delimeters[] = " \t";


static Move parse_move(char* move_string) {
    assert(move_string != NULL);
    assert(move_string[4] == '\0' || move_string[5] == '\0');

    static const MoveType char_to_promotion[] = {['n'] = MOVE_TYPE_KNIGHT_PROMOTION,
                                                 ['b'] = MOVE_TYPE_BISHOP_PROMOTION,
                                                 ['r'] = MOVE_TYPE_ROOK_PROMOTION,
                                                 ['q'] = MOVE_TYPE_QUEEN_PROMOTION};

    Square source      = coordinate_square(char_to_file(move_string[0]), char_to_rank(move_string[1]));
    Square destination = coordinate_square(char_to_file(move_string[2]), char_to_rank(move_string[3]));
    MoveType move_type = (MoveType)((move_string[4] == '\0') ? MOVE_TYPE_NORMAL
                                                             : char_to_promotion[(int)move_string[4]]);

    return new_move(source, destination, move_type);
}

void print_move(FILE* stream, Move move) {
    assert(stream != NULL);
    assert(is_valid_move(move));

    // clang-format off
    static const char square_to_string[SQUARE_COUNT][3] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };
    // clang-format on

    fprintf(stream, "%s%s", square_to_string[move_source(move)], square_to_string[move_destination(move)]);

    if (move_type(move) == MOVE_TYPE_PROMOTION)
        fputc(promotion_to_char(move), stream);
}


static void uci_id() {
    puts(
    "id name Windmolen\n"
    "id author Pieter te Brake\n");
}

static void uci_options() {
    puts(
    "option name Hash type spin default 1 min 1 max 1\n"
    "option name Threads type spin default 1 min 1 max 1\n"
    "option name MultiPV type spin default 1 min 1 max 1");
}

void uci_best_move(Move best_move) {
    assert(is_valid_move(best_move));

    printf("bestmove ");
    print_move(stdout, best_move);
    putc('\n', stdout);
    fflush(stdout);
}

static void handle_position(struct Engine* engine) {
    assert(engine != NULL);

    // strtok() has already been 'initialized' in the main UCI loop.
    char* argument = strtok(NULL, delimeters);

    const char* fen_string_end = NULL;
    if (strcmp(argument, "fen") == 0) {
        char* fen_string = argument + strlen(argument) + 1;  // Move to first character after terminator.
        while (isspace(*fen_string))
            ++fen_string;  // Move pointer to start of fen string.
        fen_string_end = position_from_FEN(&engine->position, fen_string);
    } else if (strcmp(argument, "startpos") == 0) {
        position_from_startpos(&engine->position);
    }

    argument = strtok((char*)fen_string_end, delimeters);
    if (argument == NULL)
        return;

    if (strcmp(argument, "moves") == 0) {
        argument = strtok(NULL, delimeters);
        while (argument != NULL) {
            do_move(&engine->position, parse_move(argument));
            argument = strtok(NULL, delimeters);
        }
    }
}

static void handle_go(struct Engine* engine) {
    assert(engine != NULL);

    struct SearchArguments* search_arguments = &engine->search_arguments;
    struct TimeManager* time_manager         = &engine->thread_pool.time_manager;

    *search_arguments           = (struct SearchArguments){0};
    search_arguments->max_depth = MAX_SEARCH_DEPTH;

    // strtok() has already been 'initialized' in the main UCI loop.
    char* argument = strtok(NULL, delimeters);

    while (argument != NULL) {
        if (strcmp(argument, "searchmoves") == 0) {
            argument = strtok(NULL, delimeters);

            while (argument != NULL) {
                search_arguments->search_moves[search_arguments->search_move_count] = parse_move(argument);
                ++search_arguments->search_move_count;

                argument = strtok(NULL, delimeters);
            }
        } else if (strcmp(argument, "ponder") == 0) {
            search_arguments->ponder = true;
        } else if (strcmp(argument, "wtime") == 0) {
            argument                 = strtok(NULL, delimeters);
            time_manager->white_time = (uint64_t)strtoull(argument, NULL, 10) * 1000;
        } else if (strcmp(argument, "btime") == 0) {
            argument                 = strtok(NULL, delimeters);
            time_manager->black_time = (uint64_t)strtoull(argument, NULL, 10) * 1000;
        } else if (strcmp(argument, "winc") == 0) {
            argument                      = strtok(NULL, delimeters);
            time_manager->white_increment = (uint64_t)strtoull(argument, NULL, 10) * 1000;
        } else if (strcmp(argument, "binc") == 0) {
            argument                      = strtok(NULL, delimeters);
            time_manager->black_increment = (uint64_t)strtoull(argument, NULL, 10) * 1000;
        } else if (strcmp(argument, "movestogo") == 0) {
            argument                      = strtok(NULL, delimeters);
            search_arguments->moves_to_go = (size_t)strtoull(argument, NULL, 10);
        } else if (strcmp(argument, "depth") == 0) {
            argument                    = strtok(NULL, delimeters);
            search_arguments->max_depth = (size_t)strtoull(argument, NULL, 10);
        } else if (strcmp(argument, "nodes") == 0) {
            argument                    = strtok(NULL, delimeters);
            search_arguments->max_nodes = (size_t)strtoull(argument, NULL, 10);
        } else if (strcmp(argument, "mate") == 0) {
            argument                    = strtok(NULL, delimeters);
            search_arguments->mate_in_x = (size_t)strtoull(argument, NULL, 10);
        } else if (strcmp(argument, "movetime") == 0) {
            argument                    = strtok(NULL, delimeters);
            search_arguments->move_time = (uint64_t)strtoull(argument, NULL, 10);
        } else if (strcmp(argument, "infinite") == 0) {
            search_arguments->infinite  = true;
            search_arguments->max_depth = MAX_SEARCH_DEPTH;
        } else if (strcmp(argument, "perft") == 0) {
            argument = strtok(NULL, delimeters);
            if (strcmp(argument, "ext") == 0) {
                argument     = strtok(NULL, delimeters);
                size_t depth = (size_t)strtoull(argument, NULL, 10);
                size_t extended_info[PERFT_COUNT];
                extended_perft(&engine->position, depth, extended_info);
            } else {
                size_t nodes = perft(&engine->position, (size_t)strtoull(argument, NULL, 10));
                printf("\nNodes searched: %zu\n\n", nodes);
                fflush(stdout);
            }

            return;
        } else if (strcmp(argument, "divide") == 0) {
            putc('\n', stdout);
            argument              = strtok(NULL, delimeters);
            size_t nodes_searched = divide(&engine->position, (size_t)strtoull(argument, NULL, 10));
            printf("\nNodes searched: %zu\n\n", nodes_searched);
            fflush(stdout);

            return;
        }

        argument = strtok(NULL, delimeters);
    }

    start_search(engine);
}

void uci_loop(struct Engine* engine) {
    // Minimum command support:
    //
    // uci
    // isready
    // position
    // go
    // stop
    // quit

    char line[LINE_BUFFER_SIZE];
    char* command = NULL;

    while (true) {
        if (command == NULL) {
            fgets(line, LINE_BUFFER_SIZE, stdin);
            line[strcspn(line, "\n")] = '\0';

            command = strtok(line, delimeters);
        }

        if (strcmp(command, "uci") == 0) {
            uci_id();
            uci_options();

            puts("uciok");
            fflush(stdout);
        } else if (strcmp(command, "isready") == 0) {
            initialize_engine(engine);
            construct_thread_pool(&engine->thread_pool, 1);

            puts("readyok");
            fflush(stdout);
        } else if (strcmp(command, "position") == 0) {
            handle_position(engine);
        } else if (strcmp(command, "go") == 0) {
            handle_go(engine);
        } else if (strcmp(command, "stop") == 0) {
            stop_search(engine);
        } else if (strcmp(command, "quit") == 0) {
            quit_engine(engine);
            break;
        }

        command = strtok(NULL, delimeters);
    }
}


void uci_long_info(size_t depth, size_t multipv, Score score, size_t nodes, Move best_move) {
    printf("info multipv %zu depth %zu seldepth %zu score cp %d nodes %zu pv ", multipv, depth, depth, score, nodes);
    print_move(stdout, best_move);
    putc(' ', stdout);
    putc('\n', stdout);
    fflush(stdout);
}
