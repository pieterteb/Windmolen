#include "uci.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "engine.h"
#include "move.h"
#include "options.h"
#include "perft.h"
#include "piece.h"
#include "position.h"
#include "search.h"
#include "time_manager.h"



static constexpr size_t LINE_BUFFER_SIZE = 65536;
// clang-format off
static constexpr const char DELIMETERS[] = " \t";
// clang-format on


// Parse a move from `move_string` given the current `position`.
static Move parse_move(const struct Position* position, const char* move_string) {
    assert(position != nullptr);
    assert(move_string != nullptr);
    assert(move_string[4] == '\0' || move_string[5] == '\0');

    static const enum MoveType char_to_promotion[] = {['n'] = MOVE_TYPE_KNIGHT_PROMOTION,
                                                      ['b'] = MOVE_TYPE_BISHOP_PROMOTION,
                                                      ['r'] = MOVE_TYPE_ROOK_PROMOTION,
                                                      ['q'] = MOVE_TYPE_QUEEN_PROMOTION};

    const enum Square source      = square_from_coordinates(char_to_file(move_string[0]), char_to_rank(move_string[1]));
    const enum Square destination = square_from_coordinates(char_to_file(move_string[2]), char_to_rank(move_string[3]));
    enum MoveType move_type       = MOVE_TYPE_NORMAL;

    if (move_string[4] != '\0') {
        move_type = char_to_promotion[(int)move_string[4]];
    } else if (source == king_square(position, position->side_to_move) && distance(source, destination) == 2) {
        move_type = MOVE_TYPE_CASTLE;
    } else if (destination == en_passant_square(position)
               && type_of_piece(piece_on_square(position, source)) == PIECE_TYPE_PAWN) {
        move_type = MOVE_TYPE_EN_PASSANT;
    }

    return new_move(source, destination, move_type);
}

void print_move(const Move move) {
    assert(!is_weird_move(move));

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

    if (type_of_move(move) == MOVE_TYPE_PROMOTION)
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
    // 7 is the length of the longest type + 1.
    static const char type_to_string[PIECE_TYPE_COUNT][7] = {[OPTION_TYPE_CHECK]  = "check",
                                                             [OPTION_TYPE_SPIN]   = "spin",
                                                             [OPTION_TYPE_COMBO]  = "combo",
                                                             [OPTION_TYPE_BUTTON] = "button",
                                                             [OPTION_TYPE_STRING] = "string"};

    printf("option name %s type %s default %zu min %zu max %zu\n", OPTION_THREAD_COUNT_NAME,
           type_to_string[OPTION_THREAD_COUNT_TYPE], OPTION_THREAD_COUNT_DEFAULT, OPTION_THREAD_COUNT_MIN,
           OPTION_THREAD_COUNT_MAX);
    printf("option name %s type %s default %" PRIu64 " min %" PRIu64 " max %" PRIu64 "\n", OPTION_HASH_SIZE_NAME,
           type_to_string[OPTION_HASH_SIZE_TYPE], OPTION_HASH_SIZE_DEFAULT, OPTION_HASH_SIZE_MIN, OPTION_HASH_SIZE_MAX);
    printf("option name %s type %s\n", OPTION_CLEAR_HASH_NAME, type_to_string[OPTION_CLEAR_HASH_TYPE]);
    printf("option name %s type %s default %s\n", OPTION_PONDER_MODE_NAME, type_to_string[OPTION_PONDER_MODE_TYPE],
           OPTION_PONDER_MODE_DEFAULT ? "true" : "false");
    printf("option name %s type %s default %" PRIu64 " min %" PRIu64 " max %" PRIu64 "\n", OPTION_MOVE_OVERHEAD_NAME,
           type_to_string[OPTION_MOVE_OVERHEAD_TYPE], OPTION_MOVE_OVERHEAD_DEFAULT, OPTION_MOVE_OVERHEAD_MIN,
           OPTION_MOVE_OVERHEAD_MAX);
}

void uci_best_move(const Move best_move) {
    assert(!is_weird_move(best_move));

    printf("bestmove ");
    print_move(best_move);
    putchar('\n');
}


static void handle_setoption(struct Engine* engine) {
    assert(engine != nullptr);

    // strtok() has already been 'initialized' in the main UCI loop.
    strtok(nullptr, DELIMETERS);  // Skip "name".

    char option_name[128] = "";
    const char* temp      = strtok(nullptr, DELIMETERS);  // First word of option name.
    while (temp != nullptr && strcmp(temp, "value") != 0) {
        strcat(option_name, temp);
        strcat(option_name, " ");
        temp = strtok(nullptr, DELIMETERS);
    }
    option_name[strlen(option_name) - 1] = '\0';  // Set the last space to terminator.

    if (strcmp(option_name, OPTION_THREAD_COUNT_NAME) == 0) {
        engine->options.thread_count = (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        resize_thread_pool(&engine->thread_pool, engine->options.thread_count);
    } else if (strcmp(option_name, OPTION_HASH_SIZE_NAME) == 0) {
        engine->options.hash_size = (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
    } else if (strcmp(option_name, OPTION_CLEAR_HASH_NAME) == 0) {
        // TODO: Clear hash when hash is implemented.
    } else if (strcmp(option_name, OPTION_PONDER_MODE_NAME) == 0) {
        const char* ponder_mode = strtok(nullptr, DELIMETERS);
        if (strcmp(ponder_mode, "false")) {
            engine->options.ponder_mode = false;
        } else if (strcmp(ponder_mode, "true")) {
            engine->options.ponder_mode = true;
        }
    } else if (strcmp(option_name, OPTION_MOVE_OVERHEAD_NAME) == 0) {
        engine->options.move_overhead      = 1000ULL * (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        engine->time_manager.move_overhead = engine->options.move_overhead;
    }
}

static void handle_position(struct Engine* engine) {
    assert(engine != nullptr);

    engine->info_history_count = 0;

    // strtok() has already been 'initialized' in the main UCI loop.
    const char* argument = strtok(nullptr, DELIMETERS);

    const char* fen_string = nullptr;
    if (strcmp(argument, "fen") == 0) {
        fen_string = argument + strlen(argument) + 1;  // Move to first character after terminator.
        while (isspace(*fen_string))
            ++fen_string;  // Move pointer to start of fen string.
        fen_string = setup_position_from_fen(&engine->position, &engine->info_history[engine->info_history_count++],
                                             fen_string);
    } else if (strcmp(argument, "startpos") == 0) {
        setup_start_position(&engine->position, &engine->info_history[engine->info_history_count++]);
    } else if (strcmp(argument, "kiwipete") == 0) {
        setup_kiwipete_position(&engine->position, &engine->info_history[engine->info_history_count++]);
    }

    argument = strtok((char*)fen_string, DELIMETERS);
    if (argument == nullptr)
        return;

    if (strcmp(argument, "moves") == 0) {
        argument = strtok(nullptr, DELIMETERS);
        while (argument != nullptr) {
            const Move move = parse_move(&engine->position, argument);
            do_move(&engine->position, &engine->info_history[engine->info_history_count++], move);
            argument = strtok(nullptr, DELIMETERS);
        }
    }
}

static void handle_go(struct Engine* engine) {
    assert(engine != nullptr);

    struct SearchArguments* search_arguments = &engine->search_arguments;
    struct TimeManager* time_manager         = &engine->time_manager;

    reset_time_manager(time_manager);
    reset_search_arguments(search_arguments);

    // strtok() has already been 'initialized' in the main UCI loop.
    const char* argument = strtok(nullptr, DELIMETERS);

    // All times are converted from milliseconds to microseconds.
    while (argument != nullptr) {
        if (strcmp(argument, "wtime") == 0) {
            search_arguments->infinite_search = false;
            time_manager->white_time          = 1000ULL * (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "btime") == 0) {
            search_arguments->infinite_search = false;
            time_manager->black_time          = 1000ULL * (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "winc") == 0) {
            time_manager->white_increment = 1000ULL * (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "binc") == 0) {
            time_manager->black_increment = 1000ULL * (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "ponder") == 0) {
            search_arguments->ponder = true;
        } else if (strcmp(argument, "movetime") == 0) {
            search_arguments->infinite_search = false;
            time_manager->move_time           = 1000ULL * (uint64_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "movestogo") == 0) {
            search_arguments->infinite_search = false;
            time_manager->moves_to_go         = (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "nodes") == 0) {
            // Not fully correctly implemented. The search will stop if the maximum amount of nodes is reached, but this
            // amount may be exceeded by quite a bit.
            search_arguments->infinite_search  = false;
            search_arguments->max_search_nodes = (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "depth") == 0) {
            search_arguments->infinite_search  = false;
            search_arguments->max_search_depth = (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else if (strcmp(argument, "infinite") == 0) {
            search_arguments->max_search_depth = MAX_SEARCH_DEPTH;
        } else if (strcmp(argument, "searchmoves") == 0) {
            argument = strtok(nullptr, DELIMETERS);
            while (argument != nullptr) {
                search_arguments->search_moves[search_arguments->search_move_count++] = parse_move(&engine->position,
                                                                                                   argument);
                argument                                                              = strtok(nullptr, DELIMETERS);
            }
        } else if (strcmp(argument, "mate") == 0) {
            // TODO: Not implemented yet.
            search_arguments->infinite_search = false;
            search_arguments->mate_in_x       = (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
        } else {
            if (strcmp(argument, "perft") == 0) {
                // Regular perft.
                const size_t nodes = perft(&engine->position,
                                           (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10));
                printf("Nodes searched: %zu\n", nodes);
            } else if (strcmp(argument, "extperft") == 0) {
                // Extended perth.
                struct ExtendedPerft ext_perft;
                const size_t depth = (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10);
                const size_t nodes = extended_perft(&engine->position, depth, &ext_perft);
                printf("Nodes searched:            %zu\n\n", nodes);
                printf("Captures:                  %zu\n", ext_perft.captures);
                printf("En passants:               %zu\n", ext_perft.en_passants);
                printf("Castles:                   %zu\n", ext_perft.castles);
                printf("Promotions:                %zu\n", ext_perft.promotions);
                printf("Direct checks:             %zu\n", ext_perft.direct_checks);
                printf("Single discovered checks:  %zu\n", ext_perft.single_discovered_checks);
                printf("Direct discovered checks:  %zu\n", ext_perft.direct_discovered_checks);
                printf("Double discovered checks:  %zu\n", ext_perft.double_discovered_checks);
                printf("Total checks:              %zu\n", ext_perft.direct_checks + ext_perft.single_discovered_checks
                                                           + ext_perft.direct_discovered_checks
                                                           + ext_perft.double_discovered_checks);
                printf("Direct mates:              %zu\n", ext_perft.direct_mates);
                printf("Single discovered mates:   %zu\n", ext_perft.single_discovered_mates);
                printf("Direct discovered mates:   %zu\n", ext_perft.direct_discovered_mates);
                printf("Double discovered mates:   %zu\n", ext_perft.double_discovered_mates);
                printf("Total mates:               %zu\n", ext_perft.direct_mates + ext_perft.single_discovered_mates
                                                           + ext_perft.direct_discovered_mates
                                                           + ext_perft.double_discovered_mates);
            } else if (strcmp(argument, "divide") == 0) {
                const size_t nodes_searched = divide(&engine->position,
                                                     (size_t)strtoull(strtok(nullptr, DELIMETERS), nullptr, 10));
                printf("\nNodes searched: %zu\n", nodes_searched);
            } else if (strcmp(argument, "print") == 0) {
                print_position(&engine->position);
            } else if (strcmp(argument, "fen") == 0) {
                print_fen(&engine->position);
                putchar('\n');
            }

            return;
        }

        argument = strtok(nullptr, DELIMETERS);
    }

    start_search(engine);
}

void uci_loop(struct Engine* engine) {
    assert(engine != nullptr);

    initialize_engine(engine);

    uci_startup_message();

    char line[LINE_BUFFER_SIZE];
    const char* command = nullptr;

    while (true) {
        while (command == nullptr) {
            fgets(line, LINE_BUFFER_SIZE, stdin);
            line[strcspn(line, "\n")] = ' ';  // We make sure the last character is white space. This is necessary for
                                              // option handling.
            command = strtok(line, DELIMETERS);
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
            command = strtok(nullptr, DELIMETERS);
        }

        command = strtok(nullptr, DELIMETERS);
    }
}


void uci_long_info(size_t depth, size_t multipv, Score score, size_t nodes, uint64_t time, Move best_move) {
    uint64_t time_ms = time / 1000;
    size_t nps       = (time == 0) ? 0 : 1000000 * nodes / time;

    bool mate = false;
    if (score >= MATE_SCORE - (Score)MAX_SEARCH_DEPTH) {
        mate  = true;
        score = MATE_SCORE - score;
    } else if (score <= -MATE_SCORE + (Score)MAX_SEARCH_DEPTH) {
        mate  = true;
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
