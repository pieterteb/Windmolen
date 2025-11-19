#include "uci.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "move_generation.h"
#include "options.h"
#include "perft.h"
#include "position.h"
#include "time_manager.h"
#include "types.h"



#define LINE_BUFFER_SIZE 65536


const char delimeters[] = " \t";


static Move parse_move(struct Position* position, const char* move_string) {
    assert(move_string != NULL);
    assert(move_string[4] == '\0' || move_string[5] == '\0');

    static const MoveType char_to_promotion[] = {['n'] = MOVE_TYPE_KNIGHT_PROMOTION,
                                                 ['b'] = MOVE_TYPE_BISHOP_PROMOTION,
                                                 ['r'] = MOVE_TYPE_ROOK_PROMOTION,
                                                 ['q'] = MOVE_TYPE_QUEEN_PROMOTION};

    const Square source      = square_from_coordinates(char_to_file(move_string[0]), char_to_rank(move_string[1]));
    const Square destination = square_from_coordinates(char_to_file(move_string[2]), char_to_rank(move_string[3]));
    MoveType move_type       = MOVE_TYPE_NORMAL;

    if (source == king_square(position, position->side_to_move) && abs(destination - source) == 2 * DIRECTION_EAST) {
        move_type = MOVE_TYPE_CASTLE;
    } else if (destination == position->en_passant_square) {
        move_type = MOVE_TYPE_EN_PASSANT;
    } else if (move_string[4] != '\0') {
        move_type = char_to_promotion[(int)move_string[4]];
    }

    return new_move(source, destination, move_type);
}

void print_move(Move move) {
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

    printf("%s%s", square_to_string[move_source(move)], square_to_string[move_destination(move)]);

    if (move_type(move) == MOVE_TYPE_PROMOTION)
        putchar(promotion_to_char(move));
}


static void uci_startup_message() {
    puts("Windmolen");
}

static void uci_id() {
    puts(
    "id name Windmolen\n"
    "id author Pieter te Brake\n");
}

static void uci_options() {
    printf("option name " HASH_OPTION_NAME " type spin default %" PRIu64 " min %" PRIu64 " max %" PRIu64 "\n",
           DEFAULT_HASH_SIZE, MIN_HASH_SIZE, MAX_HASH_SIZE);
    printf("option name " THREAD_OPTION_NAME " type spin default %zu min %zu max %zu\n", DEFAULT_THREAD_COUNT,
           MIN_THREAD_COUNT, MAX_THREAD_COUNT);
    printf("option name " MULTIPV_OPTION_NAME " type spin default %zu min %zu max %zu\n", DEFAULT_MULTIPV, MIN_MULTIPV,
           MAX_MULTIPV);
}

void uci_best_move(Move best_move) {
    assert(is_valid_move(best_move));

    printf("bestmove ");
    print_move(best_move);
    putchar('\n');
}


static void handle_setoption(struct Engine* engine) {
    assert(engine != NULL);

    // strtok() has already been 'initialized' in the main UCI loop.
    const char* argument = strtok(NULL, delimeters);
    assert(strcmp(argument, "name") == 0);

    argument = strtok(NULL, delimeters);
    if (strcmp(argument, HASH_OPTION_NAME) == 0) {
        argument = strtok(NULL, delimeters);
        assert(strcmp(argument, "value") == 0);
        engine->options.hash_size = (uint64_t)strtoull(strtok(NULL, delimeters), NULL, 10);
    } else if (strcmp(argument, THREAD_OPTION_NAME) == 0) {
        argument = strtok(NULL, delimeters);
        assert(strcmp(argument, "value") == 0);
        engine->options.thread_count = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);

        construct_thread_pool(&engine->thread_pool, engine->options.thread_count);
    } else if (strcmp(argument, MULTIPV_OPTION_NAME) == 0) {
        argument = strtok(NULL, delimeters);
        assert(strcmp(argument, "value") == 0);
        engine->options.multipv = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);
    }
}

static void handle_position(struct Engine* engine) {
    assert(engine != NULL);

    // strtok() has already been 'initialized' in the main UCI loop.
    const char* argument = strtok(NULL, delimeters);

    const char* fen_string = NULL;
    if (strcmp(argument, "fen") == 0) {
        fen_string = argument + strlen(argument) + 1;  // Move to first character after terminator.
        while (isspace(*fen_string))
            ++fen_string;  // Move pointer to start of fen string.
        fen_string = setup_position_from_fen(&engine->position, fen_string);
    } else if (strcmp(argument, "startpos") == 0) {
        setup_start_position(&engine->position);
    } else if (strcmp(argument, "kiwipete") == 0) {
        setup_kiwipete_position(&engine->position);
    }

    argument = strtok((char*)fen_string, delimeters);
    if (argument == NULL)
        return;

    if (strcmp(argument, "moves") == 0) {
        argument = strtok(NULL, delimeters);
        while (argument != NULL) {
            const Move move = parse_move(&engine->position, argument);
            do_move(&engine->position, move);
            argument = strtok(NULL, delimeters);
        }
    }
}

static void handle_go(struct Engine* engine) {
    assert(engine != NULL);

    struct SearchArguments* search_arguments = &engine->search_arguments;
    struct TimeManager* time_manager         = &engine->time_manager;

    reset_time_manager(time_manager);
    reset_search_arguments(search_arguments);

    // strtok() has already been 'initialized' in the main UCI loop.
    const char* argument = strtok(NULL, delimeters);

    // All times are converted from milliseconds to microseconds.
    while (argument != NULL) {
        if (strcmp(argument, "wtime") == 0) {
            search_arguments->infinite = false;
            time_manager->white_time   = 1000ULL * (uint64_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "btime") == 0) {
            search_arguments->infinite = false;
            time_manager->black_time   = 1000ULL * (uint64_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "winc") == 0) {
            time_manager->white_increment = 1000ULL * (uint64_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "binc") == 0) {
            time_manager->black_increment = 1000ULL * (uint64_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "ponder") == 0) {
            search_arguments->ponder = true;
        } else if (strcmp(argument, "movetime") == 0) {
            search_arguments->infinite = false;
            time_manager->move_time    = 1000ULL * (uint64_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "movestogo") == 0) {
            time_manager->moves_to_go = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "nodes") == 0) {
            // Not fully correctly implemented. The search will stop if the maximum amount of nodes is reached, but this
            // amount may be exceeded by quite a bit.
            search_arguments->infinite  = false;
            search_arguments->max_nodes = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "depth") == 0) {
            search_arguments->infinite  = false;
            search_arguments->max_depth = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else if (strcmp(argument, "infinite") == 0) {
            search_arguments->max_depth = MAX_SEARCH_DEPTH;
        } else if (strcmp(argument, "searchmoves") == 0) {
            argument = strtok(NULL, delimeters);
            while (argument != NULL) {
                search_arguments->search_moves[search_arguments->search_move_count++] = parse_move(&engine->position,
                                                                                                   argument);
                argument                                                              = strtok(NULL, delimeters);
            }
        } else if (strcmp(argument, "mate") == 0) {
            // Not implemented yet.
            search_arguments->infinite  = false;
            search_arguments->mate_in_x = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);
        } else {
            if (strcmp(argument, "perft") == 0) {
                argument = strtok(NULL, delimeters);
                if (strcmp(argument, "ext") == 0) {
                    // Extended perth.
                    const size_t depth = (size_t)strtoull(strtok(NULL, delimeters), NULL, 10);
                    size_t extended_info[PERFT_COUNT];
                    const size_t nodes = extended_perft(&engine->position, depth, extended_info);
                    printf("Nodes searched: %zu\n", nodes);
                } else {
                    // Regular perft.
                    const size_t nodes = perft(&engine->position, (size_t)strtoull(argument, NULL, 10));
                    printf("Nodes searched: %zu\n", nodes);
                }
            } else if (strcmp(argument, "divide") == 0) {
                const size_t nodes_searched = divide(&engine->position,
                                                     (size_t)strtoull(strtok(NULL, delimeters), NULL, 10));
                printf("\nNodes searched: %zu\n", nodes_searched);
            } else if (strcmp(argument, "print") == 0) {
                print_position(&engine->position);
            } else if (strcmp(argument, "fen") == 0) {
                print_fen(&engine->position);
                putchar('\n');
            }

            return;
        }

        argument = strtok(NULL, delimeters);
    }

    start_search(engine);
}

void uci_loop(struct Engine* engine) {
    assert(engine != NULL);

    initialize_engine(engine);
    
    uci_startup_message();

    char line[LINE_BUFFER_SIZE];
    const char* command = NULL;

    while (true) {
        if (command == NULL) {
            fgets(line, LINE_BUFFER_SIZE, stdin);
            line[strcspn(line, "\n")] = '\0';
            command                   = strtok(line, delimeters);
        }

        if (strcmp(command, "go") == 0) {
            handle_go(engine);
        } else if (strcmp(command, "stop") == 0) {
            stop_search(engine);
        } else if (strcmp(command, "ponderhit") == 0) {
            // Pondering not implemented yet so do nothing.
        } else if (strcmp(command, "position") == 0) {
            handle_position(engine);
        } else if (strcmp(command, "isready") == 0) {
            puts("readyok");
            fflush(stdout);
        } else if (strcmp(command, "ucinewgame") == 0) {
            // We currently do not need to do anything to reset the game state.
        } else if (strcmp(command, "setoption") == 0) {
            handle_setoption(engine);
        } else if (strcmp(command, "uci") == 0) {
            uci_id();
            uci_options();
            puts("uciok");
            fflush(stdout);
        } else if (strcmp(command, "quit") == 0) {
            quit_engine(engine);
            break;
        } else if (strcmp(command, "debug") == 0) {
            // We have no debug mode so consume the on/off token and do nothing.
            command = strtok(NULL, delimeters);
        }

        command = strtok(NULL, delimeters);
    }
}


void uci_long_info(size_t depth, size_t multipv, Score score, size_t nodes, uint64_t time, Move best_move) {
    uint64_t time_ms = time / 1000;
    size_t nps       = (time == 0) ? 0 : 1000000 * nodes / time;

    bool mate = false;
    if (score >= MATE_SCORE - MAX_SEARCH_DEPTH) {
        mate = true;
        score = MATE_SCORE - score;
    } else if (score <= -MATE_SCORE + MAX_SEARCH_DEPTH) {
        mate = true;
        score = -MATE_SCORE - score;
    }

    printf("info multipv %zu ", multipv);
    printf("depth %zu ", depth);
    printf("seldepth %zu ", depth);
    printf(mate ? "score mate %d " : "score cp %d ", score);
    printf("nodes %zu ", nodes);
    printf("nps %zu ", nps);
    printf("tbhits 0 ");
    printf("time %" PRIu64 " ", time_ms);
    printf("pv ");
    print_move(best_move);
    putchar('\n');
}
