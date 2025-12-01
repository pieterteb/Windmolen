#include "position.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "piece.h"
#include "util.h"
#include "zobrist.h"



static const char start_position_fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const char kiwipete_fen[]       = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";


// Returns a bitboard of all pieces that put the king of `color` in attack.
static INLINE Bitboard compute_checkers(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return attackers_of_square(position, king_square(position, color), position->total_occupancy)
         & piece_occupancy_by_color(position, opposite_color(color));
}

// Returns a bitboard of all pieces that stand between an attacking piece and the king of `color`.
static INLINE Bitboard compute_blockers(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    const enum Square king     = king_square(position, color);
    Bitboard potential_pinners = attackers_of_square(position, king, EMPTY_BITBOARD)
                               & piece_occupancy_by_color(position, opposite_color(color));

    Bitboard blockers = EMPTY_BITBOARD;
    while (potential_pinners != EMPTY_BITBOARD) {
        enum Square pinner_square   = (enum Square)pop_lsb64(&potential_pinners);
        Bitboard potential_blockers = between_bitboard(pinner_square, king)
                                    & (position->total_occupancy ^ square_bitboard(king));

        if (!popcount64_greater_than_one(potential_blockers))
            blockers |= potential_blockers;
    }

    return blockers;
}

// Returns `0` if `position` has never occured before. Else, it returns the number of plies since the previous occurence
// of `position`, or negative that number of plies if the current repetition is a threefold.
static INLINE int compute_repetition(const struct Position* position) {
    assert(position != nullptr);

    int end = (position->info->halfmove_clock < position->plies_since_start) ? (int)position->info->halfmove_clock
                                                                             : (int)position->plies_since_start;

    if (end < 4)
        return 0;

    // We start the search 4 plies back, since that is the first time a repetition can occur.
    struct PositionInfo* info = position->info->previous_info->previous_info;
    for (int i = 4; i <= end; i += 2) {
        info = info->previous_info->previous_info;
        if (info->zobrist_key == position->info->zobrist_key)
            return (info->repetition == 0) ? i : -i;
    }

    return 0;
}


// Squares between which the rook switches based on the destination of the king.
// clang-format off
static const Bitboard rook_bitboards[SQUARE_COUNT] = {
    [SQUARE_G1] = SQUARE_BITBOARD(SQUARE_F1) | SQUARE_BITBOARD(SQUARE_H1),
    [SQUARE_C1] = SQUARE_BITBOARD(SQUARE_A1) | SQUARE_BITBOARD(SQUARE_D1),
    [SQUARE_G8] = SQUARE_BITBOARD(SQUARE_F8) | SQUARE_BITBOARD(SQUARE_H8),
    [SQUARE_C8] = SQUARE_BITBOARD(SQUARE_A8) | SQUARE_BITBOARD(SQUARE_D8)
};
static const enum Square rook_sources[SQUARE_COUNT] = {
    [SQUARE_G1] = SQUARE_H1,
    [SQUARE_C1] = SQUARE_A1,
    [SQUARE_G8] = SQUARE_H8,
    [SQUARE_C8] = SQUARE_A8
};
static const enum Square rook_destinations[SQUARE_COUNT] = {
    [SQUARE_G1] = SQUARE_F1,
    [SQUARE_C1] = SQUARE_D1,
    [SQUARE_G8] = SQUARE_F8,
    [SQUARE_C8] = SQUARE_D8
};
// clang-format on

void do_move(struct Position* position, struct PositionInfo* new_info, const Move move) {
    assert(position != nullptr);
    assert(new_info != nullptr);
    assert(!is_weird_move(move));

    const enum Square source      = move_source(move);
    const enum Square destination = move_destination(move);
    const enum MoveType type      = move_type(move);
    enum Piece piece              = piece_on_square(position, source);
    const enum Color side_to_move = position->side_to_move;
    const enum Color opponent     = opposite_color(side_to_move);

    assert(piece != PIECE_NONE);
    assert(is_valid_color(side_to_move));
    assert(color_of_piece(piece) == side_to_move);
    assert(type != MOVE_TYPE_EN_PASSANT || position->info->en_passant_square != SQUARE_NONE);
    assert(popcount64(piece_occupancy(position, side_to_move, PIECE_TYPE_KING)) == 1);


    // Copy information from previous info and switch to new info. halfmove_clock might be set to 0 later.
    new_info->castling_rights   = position->info->castling_rights;
    new_info->en_passant_square = position->info->en_passant_square;
    new_info->halfmove_clock    = position->info->halfmove_clock + 1;
    new_info->previous_info     = position->info;
    position->info              = new_info;


    // Update side to move.
    position->side_to_move = opponent;
    ++position->plies_since_start;
    position->fullmove_counter += side_to_move;  // Trick to only increasy the fullmove counter after black has played
                                                 // (COLOR_WHITE == 0 and COLOR_BLACK == 1).
    position->info->zobrist_key ^= side_to_move_zobrist_key;


    // Remove the moved piece from its current square.
    remove_piece(position, piece, source);

    // After the piece has been removed for its source square, we do not need to know anymore what type of piece it was.
    // Therefore, we change it if the move is a promotion.
    if (type == MOVE_TYPE_PROMOTION)
        piece = create_piece(side_to_move, promotion_piece_type(move));

    const enum PieceType piece_type     = type_of_piece(piece);
    const Bitboard destination_bitboard = square_bitboard(destination);

    // Handle regular and en passant captures.
    const bool is_capture = piece_on_square(position, destination) != PIECE_NONE || type == MOVE_TYPE_EN_PASSANT;
    if (is_capture) {
        if (type != MOVE_TYPE_EN_PASSANT) {
            position->info->captured_piece = piece_on_square(position, destination);

            assert(color_of_piece(position->info->captured_piece) == opponent);

            position->occupancy_by_type[type_of_piece(position->info->captured_piece)] ^= destination_bitboard;
            position->occupancy_by_color[opponent] ^= destination_bitboard;
            position->info->zobrist_key ^= piece_zobrist_keys[position->info->captured_piece][destination];
        } else {
            enum Square deletion_square = (enum Square)(
            destination + ((side_to_move == COLOR_WHITE) ? DIRECTION_SOUTH : DIRECTION_NORTH));
            position->info->captured_piece = create_piece(opponent, PIECE_TYPE_PAWN);

            remove_piece_type(position, opponent, PIECE_TYPE_PAWN, deletion_square);
        }

        position->info->halfmove_clock = 0;  // Irreversible move was played.
    } else {
        position->info->captured_piece = PIECE_NONE;
    }

    if (piece_type == PIECE_TYPE_PAWN)
        position->info->halfmove_clock = 0;  // Irreversible move was played.

    // Place moved piece on destination square.
    position->piece_on_square[destination] = piece;
    position->occupancy_by_type[piece_type] |= destination_bitboard;
    position->occupancy_by_color[side_to_move] |= destination_bitboard;
    position->info->zobrist_key ^= piece_zobrist_keys[piece][destination];

    // Handle king moves.
    if (piece_type == PIECE_TYPE_KING) {
        position->king_square[side_to_move] = destination;

        if (type == MOVE_TYPE_CASTLE) {
            assert(!is_capture);
            assert(side_to_move == COLOR_BLACK || (destination == SQUARE_G1 || destination == SQUARE_C1));
            assert(side_to_move == COLOR_WHITE || (destination == SQUARE_G8 || destination == SQUARE_C8));

            const enum Square rook_source_square      = rook_sources[destination];
            const enum Square rook_destination_square = rook_destinations[destination];

            const enum Piece rook                              = create_piece(side_to_move, PIECE_TYPE_ROOK);
            position->piece_on_square[rook_destination_square] = rook;
            position->piece_on_square[rook_source_square]      = PIECE_NONE;
            position->info->zobrist_key ^= piece_zobrist_keys[rook][rook_source_square]
                                         ^ piece_zobrist_keys[rook][rook_destination_square];

            // This switches the rook bit between the rook source and destination squares. This will work since the rook
            // destination square must be empty in a legal move.
            position->occupancy_by_type[PIECE_TYPE_ROOK] ^= rook_bitboards[destination];
            position->occupancy_by_color[side_to_move] ^= rook_bitboards[destination];
        }

        // Revert zobrist hashing for current castling rights, and after updating the castling rights update the zobrist
        // hash. We use a clever trick to update the castlingright, abusing compiler LEA instruction optimization:
        // https://godbolt.org/z/qGjac46e9.
        position->info->zobrist_key ^= castle_zobrist_keys[position->info->castling_rights];
        position->info->castling_rights &= ~CASTLE_BLACK + opponent * (~CASTLE_WHITE - ~CASTLE_BLACK);
        position->info->zobrist_key ^= castle_zobrist_keys[position->info->castling_rights];
    }

    // All bitboards have been updated at this point.
    position->total_occupancy = piece_occupancy_by_color(position, COLOR_WHITE)
                              | piece_occupancy_by_color(position, COLOR_BLACK);

    // Revert zobrist hashing for current castling rights, and after updating the castling rights update the zobrist
    // hash.
    position->info->zobrist_key ^= castle_zobrist_keys[position->info->castling_rights];
    if (position->piece_on_square[SQUARE_A1] != PIECE_WHITE_ROOK)
        position->info->castling_rights &= ~CASTLE_WHITE_000;
    if (position->piece_on_square[SQUARE_H1] != PIECE_WHITE_ROOK)
        position->info->castling_rights &= ~CASTLE_WHITE_00;
    if (position->piece_on_square[SQUARE_A8] != PIECE_BLACK_ROOK)
        position->info->castling_rights &= ~CASTLE_BLACK_000;
    if (position->piece_on_square[SQUARE_H8] != PIECE_BLACK_ROOK)
        position->info->castling_rights &= ~CASTLE_BLACK_00;
    position->info->zobrist_key ^= castle_zobrist_keys[position->info->castling_rights];

    const bool is_double_pawn_push = piece_type == PIECE_TYPE_PAWN && distance(source, destination) == 2;
    // We always undo the en passant zobrist key if there was an en passant square in the previous position. We either
    // get a new en passant square this move or we do not have an en passant square at all.
    if (position->info->en_passant_square != SQUARE_NONE)
        position->info->zobrist_key ^= en_passant_zobrist_keys[file_of_square(position->info->en_passant_square)];
    if (is_double_pawn_push) {
        position->info->en_passant_square = (enum Square)(
        source + ((side_to_move == COLOR_WHITE) ? DIRECTION_NORTH : DIRECTION_SOUTH));
        position->info->zobrist_key ^= en_passant_zobrist_keys[file_of_square(position->info->en_passant_square)];
    } else {
        position->info->en_passant_square = SQUARE_NONE;
    }

    // At this point, the Zobrist key has been calculated so we can update repetition.
    position->info->repetition = compute_repetition(position);

    position->info->checkers               = compute_checkers(position, opponent);
    position->info->blockers[side_to_move] = compute_blockers(position, side_to_move);
    position->info->blockers[opponent]     = compute_blockers(position, opponent);
}

void undo_move(struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));

    assert(position->info->previous_info != nullptr);

    const enum Square source        = move_source(move);
    const enum Square destination   = move_destination(move);
    const enum MoveType type        = move_type(move);
    enum Piece piece                = piece_on_square(position, destination);
    const enum Color opponent       = position->side_to_move;
    const enum Color side_to_move   = opposite_color(opponent);
    const enum Piece captured_piece = position->info->captured_piece;


    assert(piece != PIECE_NONE);
    assert(is_valid_color(side_to_move));
    assert(color_of_piece(piece) == side_to_move);
    assert(popcount64(piece_occupancy(position, side_to_move, PIECE_TYPE_KING)) == 1);

    position->side_to_move = side_to_move;
    --position->plies_since_start;
    position->fullmove_counter -= opponent;

    if (type == MOVE_TYPE_CASTLE) {
        const enum Square king = king_square(position, side_to_move);

        // Switch the rook back.
        position->occupancy_by_type[PIECE_TYPE_ROOK] ^= rook_bitboards[king];
        position->occupancy_by_color[side_to_move] ^= rook_bitboards[king];
        position->piece_on_square[rook_sources[king]]      = create_piece(side_to_move, PIECE_TYPE_ROOK);
        position->piece_on_square[rook_destinations[king]] = PIECE_NONE;
    }

    const Bitboard destination_bitboard    = square_bitboard(destination);
    position->piece_on_square[destination] = PIECE_NONE;
    position->occupancy_by_type[type_of_piece(piece)] ^= destination_bitboard;
    position->occupancy_by_color[color_of_piece(piece)] ^= destination_bitboard;

    if (type == MOVE_TYPE_PROMOTION)
        piece = create_piece(side_to_move, PIECE_TYPE_PAWN);

    const Bitboard source_bitboard    = square_bitboard(source);
    position->piece_on_square[source] = piece;
    position->occupancy_by_type[type_of_piece(piece)] |= source_bitboard;
    position->occupancy_by_color[color_of_piece(piece)] |= source_bitboard;

    if (type_of_piece(piece) == PIECE_TYPE_KING)
        position->king_square[side_to_move] = source;

    if (type == MOVE_TYPE_EN_PASSANT) {
        assert(type_of_piece(captured_piece) == PIECE_TYPE_PAWN);

        const enum Square pawn_square = destination
                                      + (enum Square)((side_to_move == COLOR_WHITE) ? DIRECTION_SOUTH
                                                                                    : DIRECTION_NORTH);
        position->piece_on_square[pawn_square] = captured_piece;
        position->occupancy_by_type[PIECE_TYPE_PAWN] |= square_bitboard(pawn_square);
        position->occupancy_by_color[opponent] |= square_bitboard(pawn_square);
    } else if (captured_piece != PIECE_NONE) {
        position->piece_on_square[destination] = captured_piece;
        position->occupancy_by_type[type_of_piece(captured_piece)] |= destination_bitboard;
        position->occupancy_by_color[color_of_piece(captured_piece)] |= destination_bitboard;
    }

    position->total_occupancy = piece_occupancy_by_color(position, COLOR_WHITE)
                              | piece_occupancy_by_color(position, COLOR_BLACK);

    position->info = position->info->previous_info;
}


void setup_start_position(struct Position* position, struct PositionInfo* info) {
    assert(position != nullptr);
    assert(info != nullptr);

    setup_position_from_fen(position, info, start_position_fen);
}

void setup_kiwipete_position(struct Position* position, struct PositionInfo* info) {
    assert(position != nullptr);
    assert(info != nullptr);

    setup_position_from_fen(position, info, kiwipete_fen);
}

const char* setup_position_from_fen(struct Position* position, struct PositionInfo* info, const char* fen) {
    assert(position != nullptr);
    assert(info != nullptr);
    assert(fen != nullptr);

    // clang-format off
    static const enum Piece char_to_piece[] = {
        ['P'] = PIECE_WHITE_PAWN,   ['p'] = PIECE_BLACK_PAWN,
        ['N'] = PIECE_WHITE_KNIGHT, ['n'] = PIECE_BLACK_KNIGHT,
        ['B'] = PIECE_WHITE_BISHOP, ['b'] = PIECE_BLACK_BISHOP,
        ['R'] = PIECE_WHITE_ROOK,   ['r'] = PIECE_BLACK_ROOK,
        ['Q'] = PIECE_WHITE_QUEEN,  ['q'] = PIECE_BLACK_QUEEN,
        ['K'] = PIECE_WHITE_KING,   ['k'] = PIECE_BLACK_KING
    };
    // clang-format on

    static_assert(offsetof(struct Position, piece_on_square) + sizeof(((struct Position*)0)->piece_on_square)
                  == sizeof(struct Position),
                  "piece_on_square must be the last member of struct Position.");
    memset(position, 0, offsetof(struct Position, piece_on_square));
    static_assert(sizeof(enum PieceType) == 1U, "memset() requires byte size array elements.");
    memset(&position->piece_on_square, PIECE_NONE, sizeof(position->piece_on_square));

    memset(info, 0, sizeof(*info));
    info->captured_piece = PIECE_NONE;
    position->info       = info;

    // Parse board configuration.
    enum Square square = SQUARE_A8;
    char current_char;
    while (!isspace(*fen)) {
        current_char = *fen++;
        if (isdigit(current_char)) {
            square += (enum Square)((current_char - '0') * DIRECTION_EAST);
        } else if (current_char == '/') {
            square += 2 * DIRECTION_SOUTH;
        } else {
            enum Piece piece = char_to_piece[(int)current_char];
            place_piece(position, piece, square);

            position->info->zobrist_key ^= piece_zobrist_keys[piece][square];

            if (piece == PIECE_WHITE_KING)
                position->king_square[COLOR_WHITE] = square;
            if (piece == PIECE_BLACK_KING)
                position->king_square[COLOR_BLACK] = square;

            square += DIRECTION_EAST;
        }
    }
    ++fen;  // Skip space.

    // Parse side to move.
    const enum Color side_to_move = (*fen++ == 'w') ? COLOR_WHITE : COLOR_BLACK;
    position->side_to_move        = side_to_move;
    static_assert(COLOR_WHITE == 0, "COLOR_WHITE should have a value of 0.");
    position->info->zobrist_key ^= (ZobristKey)side_to_move * side_to_move_zobrist_key;
    ++fen;  // Skip space.

    // Parse castling rights.
    do {
        switch (*fen++) {
            case 'K':
                position->info->castling_rights |= CASTLE_WHITE_00;
                break;
            case 'Q':
                position->info->castling_rights |= CASTLE_WHITE_000;
                break;
            case 'k':
                position->info->castling_rights |= CASTLE_BLACK_00;
                break;
            case 'q':
                position->info->castling_rights |= CASTLE_BLACK_000;
                break;
            case '-':
                // Do nothing.
                break;
        }
    } while (!isspace(*fen));
    position->info->zobrist_key ^= castle_zobrist_keys[position->info->castling_rights];
    ++fen;  // Skip space.

    // Parse en passant square.
    if (*fen != '-') {
        enum File file                    = char_to_file(*fen++);
        enum Rank rank                    = char_to_rank(*fen++);
        position->info->en_passant_square = square_from_coordinates(file, rank);
        position->info->zobrist_key ^= en_passant_zobrist_keys[file];
    } else {
        position->info->en_passant_square = SQUARE_NONE;
        ++fen;
    }
    ++fen;  // Skip space.

    // Parse halfmove clock.
    size_t temp = 0;
    while (isdigit(*fen))
        temp = temp * 10 + (size_t)(*fen++ - '0');
    position->info->halfmove_clock = temp;
    ++fen;  // Skip space.

    // Parse fullmove counter.
    temp = 0;
    while (isdigit(*fen))
        temp = temp * 10 + (size_t)(*fen++ - '0');
    position->fullmove_counter = temp;

    // Compute remaining tables.
    position->total_occupancy = position->occupancy_by_color[COLOR_WHITE] | position->occupancy_by_color[COLOR_BLACK];

    position->info->blockers[COLOR_WHITE] = compute_blockers(position, COLOR_WHITE);
    position->info->blockers[COLOR_BLACK] = compute_blockers(position, COLOR_BLACK);
    position->info->checkers              = compute_checkers(position, side_to_move);

    return fen;
}


// clang-format off
static const int piece_to_char[] = {
    [PIECE_WHITE_PAWN]   = 'P', [PIECE_BLACK_PAWN]   = 'p',
    [PIECE_WHITE_KNIGHT] = 'N', [PIECE_BLACK_KNIGHT] = 'n',
    [PIECE_WHITE_BISHOP] = 'B', [PIECE_BLACK_BISHOP] = 'b',
    [PIECE_WHITE_ROOK]   = 'R', [PIECE_BLACK_ROOK]   = 'r',
    [PIECE_WHITE_QUEEN]  = 'Q', [PIECE_BLACK_QUEEN]  = 'q',
    [PIECE_WHITE_KING]   = 'K', [PIECE_BLACK_KING]   = 'k',

    [PIECE_NONE] = ' '
};
// clang-format on

void print_fen(const struct Position* position) {
    assert(position != NULL);

    // Print board.
    size_t empty_squares;
    for (enum Rank rank = RANK_8; rank < RANK_COUNT; --rank) {
        empty_squares = 0;
        for (enum File file = FILE_A; file <= FILE_H; ++file) {
            enum Square square = square_from_coordinates(file, rank);

            if (piece_on_square(position, square) == PIECE_NONE) {
                ++empty_squares;
            } else {
                if (empty_squares != 0) {
                    putchar('0' + (char)empty_squares);
                    empty_squares = 0;
                }
                putchar(piece_to_char[piece_on_square(position, square)]);
            }
        }

        if (empty_squares != 0)
            putchar('0' + (char)empty_squares);
        if (rank != RANK_1)
            putchar('/');
    }

    // Print side to move.
    printf((position->side_to_move == COLOR_WHITE) ? " w " : " b ");

    // Print castling rights.
    if (position->info->castling_rights == CASTLE_NONE) {
        printf("- ");
    } else {
        if (position->info->castling_rights & CASTLE_WHITE_00)
            putchar('K');
        if (position->info->castling_rights & CASTLE_WHITE_000)
            putchar('Q');
        if (position->info->castling_rights & CASTLE_BLACK_00)
            putchar('k');
        if (position->info->castling_rights & CASTLE_BLACK_000)
            putchar('q');
        putchar(' ');
    }

    // Print en passant square.
    if (position->info->en_passant_square != SQUARE_NONE) {
        putchar('a' + (char)file_of_square(position->info->en_passant_square));
        putchar('1' + (char)file_of_square(position->info->en_passant_square));
    } else {
        putchar('-');
    }

    // Print halfmove clock and fullmove counter.
    printf(" %zu %zu", position->info->halfmove_clock, position->fullmove_counter);
}

void print_position(const struct Position* position) {
    assert(position != nullptr);

    puts("+---+---+---+---+---+---+---+---+");

    for (enum Rank rank = RANK_8; rank < RANK_COUNT; --rank) {
        for (enum File file = FILE_A; file <= FILE_H; ++file)
            printf("| %c ", piece_to_char[piece_on_square(position, square_from_coordinates(file, rank))]);

        static_assert(IS_SAME_TYPE(enum Rank, uint8_t), "Wrong format specifier used.");
        printf("| %" PRIu8 "\n+---+---+---+---+---+---+---+---+\n", (enum Rank)(rank + 1));
    }

    printf(
    "  a   b   c   d   e   f   g   h\n"
    "FEN: ");
    print_fen(position);
    printf("\nZobrist Hash: 0x%016" PRIx64 "\n", position->info->zobrist_key);
}
