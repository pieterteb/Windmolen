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



// Returns a bitboard of all pieces that put the king of `color` in attack.
static INLINE Bitboard compute_checkers(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    // The checkers are just the oppenent pieces that attack the friendly king square.
    return attackers_of_square(position, king_square(position, color), position->total_occupancy)
         & piece_occupancy_by_color(position, opposite_color(color));
}

// Returns a bitboard of all pieces in `position` that stand between an attacking piece and the king of `color`.
static INLINE Bitboard compute_blockers(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    const enum Square king = king_square(position, color);

    // The only pieces that can pin other pieces to the king are slider pieces, i.e. bishops, rooks and queens. We use
    // the fact that a piece attacks the king square if that same piece would attack itself from the king square.
    const Bitboard bishop_potential_pinners = piece_base_attacks(PIECE_TYPE_BISHOP, king)
                                            & bishop_queen_occupancy_by_type(position);
    const Bitboard rook_potential_pinners = piece_base_attacks(PIECE_TYPE_ROOK, king)
                                          & rook_queen_occupancy_by_type(position);
    Bitboard potential_pinners = (bishop_potential_pinners | rook_potential_pinners)
                               & piece_occupancy_by_color(position, opposite_color(color));

    // Our king is not a blocker.
    const Bitboard blocker_mask = position->total_occupancy ^ square_bitboard(king);
    Bitboard blockers           = EMPTY_BITBOARD;
    while (potential_pinners != EMPTY_BITBOARD) {
        const enum Square pinner_square   = (enum Square)pop_lsb64(&potential_pinners);
        const Bitboard potential_blockers = between_bitboard(pinner_square, king) & blocker_mask;

        // If there are not more than 1 piece in between the pinner and the king, that piece is a blocker.
        if (!popcount64_greater_than_one(potential_blockers))
            blockers |= potential_blockers;
    }

    return blockers;
}

// Returns `0` if `position` has never occured before. Else, it returns the number of plies since the previous occurence
// of `position`, or negative that number of plies if the current repetition is a threefold.
static INLINE int compute_repetition(const struct Position* position) {
    assert(position != nullptr);

    // Search until the last reversible move that has been played since the start of the known game history. This might
    // be relevant if the FEN of the initial position had a non-zero halfmove clock.
    const int end = (position->info->halfmove_clock < position->plies_since_start) ? (int)position->info->halfmove_clock
                                                                                   : (int)position->plies_since_start;

    if (end < 4)
        return 0;  // A repetition can only occur after 4+ plies.

    // We start the search 4 plies back, since that is the first time a repetition can occur.
    const ZobristKey current_key = zobrist_key(position);
    struct PositionInfo* info    = position->info->previous_info->previous_info;

    int i = 4;
    do {
        info = info->previous_info->previous_info;
        if (info->zobrist_key == current_key)
            return (info->repetition == 0) ? i : -i;

        i += 2;
    } while (i <= end);

    return 0;
}


// Squares between which the rook switches based on the destination of the king.
// clang-format off
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

static INLINE void do_castling(struct Position* position, const enum Square source, const enum Square destination) {
    assert(position != nullptr);
    assert(is_valid_square(source));
    assert(is_valid_square(destination));

    const enum Square rook_source      = rook_sources[destination];
    const enum Square rook_destination = rook_destinations[destination];

    remove_piece(position, source);
    remove_piece(position, rook_source);
    place_piece(position, create_piece(position->side_to_move, PIECE_TYPE_KING), destination);
    place_piece(position, create_piece(position->side_to_move, PIECE_TYPE_ROOK), rook_destination);
}

static INLINE void undo_castling(struct Position* position, const enum Square source, const enum Square destination) {
    assert(position != nullptr);
    assert(is_valid_square(source));
    assert(is_valid_square(destination));

    const enum Square rook_source      = rook_sources[destination];
    const enum Square rook_destination = rook_destinations[destination];
    const enum Color opponent          = opposite_color(position->side_to_move);

    remove_piece(position, destination);
    remove_piece(position, rook_destination);
    place_piece(position, create_piece(opponent, PIECE_TYPE_KING), source);
    place_piece(position, create_piece(opponent, PIECE_TYPE_ROOK), rook_source);
}

void do_move(struct Position* position, struct PositionInfo* new_info, const Move move) {
    assert(position != nullptr);
    assert(new_info != nullptr);
    assert(new_info != position->info);
    assert(!is_weird_move(move));

    ZobristKey zobrist_key = position->info->zobrist_key ^ side_to_move_zobrist_key;

    // Copy information from previous info and switch to new info. new_info will be used to update position->info fields
    // later in this function.
    memcpy(new_info, position->info, offsetof(struct PositionInfo, previous_info));
    new_info->previous_info = position->info;
    position->info          = new_info;

    const enum Color side_to_move = position->side_to_move;
    const enum Color opponent     = opposite_color(side_to_move);

    // Increment ply counters.
    ++new_info->halfmove_clock;  // Might be set to 0 later on.
    ++position->plies_since_start;
    position->fullmove_counter += side_to_move;  // Trick to only increase the fullmove counter after black has played
                                                 // (COLOR_WHITE == 0 and COLOR_BLACK == 1).

    const enum MoveType move_type   = type_of_move(move);
    const enum Square source        = move_source(move);
    const enum Square destination   = move_destination(move);
    enum Piece piece                = piece_on_square(position, source);
    const enum Piece captured_piece = (move_type == MOVE_TYPE_EN_PASSANT) ? create_piece(opponent, PIECE_TYPE_PAWN)
                                                                          : piece_on_square(position, destination);

    // Set captured piece.
    new_info->captured_piece = captured_piece;

    assert(piece != PIECE_NONE);
    assert(is_valid_color(side_to_move));
    assert(color_of_piece(piece) == side_to_move);
    assert(move_type != MOVE_TYPE_EN_PASSANT || en_passant_square(position) != SQUARE_NONE);


    if (move_type == MOVE_TYPE_CASTLE) {
        assert(piece == create_piece(side_to_move, PIECE_TYPE_KING));
        assert(captured_piece == PIECE_NONE);

        do_castling(position, source, destination);

        // Update Zobrist key for the rook displacement, the king will be accounted for later.
        const enum Piece rook = create_piece(side_to_move, PIECE_TYPE_ROOK);
        zobrist_key ^= piece_zobrist_keys[rook][rook_sources[destination]]
                     ^ piece_zobrist_keys[rook][rook_destinations[destination]];
    } else if (captured_piece != PIECE_NONE) {
        enum Square captured_square = destination;

        if (move_type == MOVE_TYPE_EN_PASSANT) {
            captured_square = square_step(destination,
                                          (side_to_move == COLOR_WHITE) ? DIRECTION_SOUTH : DIRECTION_NORTH);

            // Normal captures will be updated later.
            remove_piece(position, captured_square);
        }

        // Update the Zobrist key for the captured piece.
        zobrist_key ^= piece_zobrist_keys[captured_piece][captured_square];

        new_info->halfmove_clock = 0;  // Irreversible move was played.
    }

    // Reset the en passant square and update the Zobrist key.
    if (new_info->en_passant_square != SQUARE_NONE) {
        zobrist_key ^= en_passant_zobrist_keys[file_of_square(new_info->en_passant_square)];
        new_info->en_passant_square = SQUARE_NONE;
    }

    // Update Zobrist key for moved piece.
    zobrist_key ^= piece_zobrist_keys[piece][source];

    if (type_of_piece(piece) == PIECE_TYPE_PAWN) {
        // Clever trick to detect a double pawn push.
        if (((int)source ^ (int)destination) == 16) {
            // Update en passant square.
            new_info->en_passant_square = square_step(
            source, (side_to_move == COLOR_WHITE) ? DIRECTION_NORTH : DIRECTION_SOUTH);
            zobrist_key ^= en_passant_zobrist_keys[file_of_square(new_info->en_passant_square)];
        }

        if (move_type == MOVE_TYPE_PROMOTION)
            piece = create_piece(side_to_move, promotion_piece_type(move));  // Change piece in case of promotion.

        new_info->halfmove_clock = 0;  // Irreversible move was played.
    } else if (type_of_piece(piece) == PIECE_TYPE_KING) {
        // Update the king square.
        position->king_square[side_to_move] = destination;
    }

    zobrist_key ^= piece_zobrist_keys[piece][destination];

    // clang-format off
    static const enum CastlingRights castling_rights_mask[SQUARE_COUNT] = {
        [SQUARE_A1] = CASTLE_WHITE_000,
        [SQUARE_H1] = CASTLE_WHITE_00,
        [SQUARE_A8] = CASTLE_BLACK_000,
        [SQUARE_H8] = CASTLE_BLACK_00,
        [SQUARE_E1] = CASTLE_WHITE,
        [SQUARE_E8] = CASTLE_BLACK
    };
    // clang-format on

    // Update the castling rights if necessary.
    if (new_info->castling_rights & (castling_rights_mask[source] | castling_rights_mask[destination])) {
        // We first undo the current castle zobrist key, and then apply the new one.
        zobrist_key ^= castle_zobrist_keys[new_info->castling_rights];
        new_info->castling_rights &= ~(castling_rights_mask[source] | castling_rights_mask[destination]);
        zobrist_key ^= castle_zobrist_keys[new_info->castling_rights];
    }

    // We move the piece if not a castle move. Castling and en passant have been handled earlier. Zobrist keys have
    // already been updated.
    if (move_type != MOVE_TYPE_CASTLE) {
        remove_piece(position, source);  // Remove the piece from the source square.

        if (captured_piece != PIECE_NONE && move_type != MOVE_TYPE_EN_PASSANT)
            replace_piece(position, piece, destination);
        else
            place_piece(position, piece, destination);
    }

    // All bitboards have been updated at this point.
    position->total_occupancy = piece_occupancy_by_color(position, COLOR_WHITE)
                              | piece_occupancy_by_color(position, COLOR_BLACK);

    new_info->checkers               = compute_checkers(position, opponent);
    new_info->blockers[side_to_move] = compute_blockers(position, side_to_move);
    new_info->blockers[opponent]     = compute_blockers(position, opponent);

    // Update side to move and new Zobrist key.
    position->side_to_move = opponent;
    new_info->zobrist_key  = zobrist_key;

    // At this point, the Zobrist key has been calculated so we can update repetition.
    new_info->repetition = compute_repetition(position);
}

void undo_move(struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));
    assert(position->info->previous_info != nullptr);

    const enum Color side_to_move = position->side_to_move;
    const enum Color opponent     = opposite_color(side_to_move);

    // Decrement ply counters.
    --position->plies_since_start;
    position->fullmove_counter -= opponent;  // Trick to only decrease the fullmove counter if white is to move
                                             // (COLOR_WHITE == 0 and COLOR_BLACK == 1).

    const enum MoveType move_type   = type_of_move(move);
    const enum Square source        = move_source(move);
    const enum Square destination   = move_destination(move);
    enum Piece piece                = piece_on_square(position, destination);
    const enum Piece captured_piece = position->info->captured_piece;

    assert(piece != PIECE_NONE);
    assert(is_valid_color(side_to_move));
    assert(color_of_piece(piece) == opponent);


    if (move_type == MOVE_TYPE_CASTLE) {
        assert(piece == create_piece(opponent, PIECE_TYPE_KING));
        assert(captured_piece == PIECE_NONE);

        // Handle castling.
        undo_castling(position, source, destination);
    } else if (move_type == MOVE_TYPE_EN_PASSANT) {
        const enum Square captured_square = square_step(destination,
                                                        (opponent == COLOR_WHITE) ? DIRECTION_SOUTH : DIRECTION_NORTH);

        // Normal captures will be updated later.
        place_piece(position, captured_piece, captured_square);
    }

    // We move the piece back if not a castle move. Castling and en passant capture have been handled earlier.
    if (move_type != MOVE_TYPE_CASTLE) {
        move_piece(position, destination, source);  // Move the piece back.

        if (captured_piece != PIECE_NONE && move_type != MOVE_TYPE_EN_PASSANT)
            place_piece(position, captured_piece, destination);  // Place captured piece back.

        // Demote to pawn in case of promotion move.
        if (move_type == MOVE_TYPE_PROMOTION)
            replace_piece(position, create_piece(opponent, PIECE_TYPE_PAWN), source);
    }

    // Update king square.
    if (type_of_piece(piece) == PIECE_TYPE_KING)
        position->king_square[opponent] = source;

    // All bitboards have been updated at this point.
    position->total_occupancy = piece_occupancy_by_color(position, COLOR_WHITE)
                              | piece_occupancy_by_color(position, COLOR_BLACK);

    // Change to previous info.
    position->info = position->info->previous_info;

    // Update side to move.
    position->side_to_move = opponent;
}


void setup_start_position(struct Position* position, struct PositionInfo* info) {
    assert(position != nullptr);
    assert(info != nullptr);

    static const char start_position_fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    setup_position_from_fen(position, info, start_position_fen);
}

void setup_kiwipete_position(struct Position* position, struct PositionInfo* info) {
    assert(position != nullptr);
    assert(info != nullptr);

    static const char kiwipete_fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

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
    static_assert(sizeof(enum Piece) == 1U, "memset() requires byte size array elements.");
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

            info->zobrist_key ^= piece_zobrist_keys[piece][square];

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
    info->zobrist_key ^= (ZobristKey)side_to_move * side_to_move_zobrist_key;
    ++fen;  // Skip space.

    // Parse castling rights.
    do {
        switch (*fen++) {
            case 'K':
                info->castling_rights |= CASTLE_WHITE_00;
                break;
            case 'Q':
                info->castling_rights |= CASTLE_WHITE_000;
                break;
            case 'k':
                info->castling_rights |= CASTLE_BLACK_00;
                break;
            case 'q':
                info->castling_rights |= CASTLE_BLACK_000;
                break;
            case '-':
                // Do nothing.
                break;
        }
    } while (!isspace(*fen));
    info->zobrist_key ^= castle_zobrist_keys[info->castling_rights];
    ++fen;  // Skip space.

    // Parse en passant square.
    if (*fen != '-') {
        enum File file          = char_to_file(*fen++);
        enum Rank rank          = char_to_rank(*fen++);
        info->en_passant_square = square_from_coordinates(file, rank);
        info->zobrist_key ^= en_passant_zobrist_keys[file];
    } else {
        info->en_passant_square = SQUARE_NONE;
        ++fen;
    }
    ++fen;  // Skip space.

    // Parse halfmove clock.
    size_t temp = 0;
    while (isdigit(*fen))
        temp = temp * 10 + (size_t)(*fen++ - '0');
    info->halfmove_clock = temp;
    ++fen;  // Skip space.

    // Parse fullmove counter.
    temp = 0;
    while (isdigit(*fen))
        temp = temp * 10 + (size_t)(*fen++ - '0');
    position->fullmove_counter = temp;

    // Compute remaining tables.
    position->total_occupancy = piece_occupancy_by_color(position, COLOR_WHITE)
                              | piece_occupancy_by_color(position, COLOR_BLACK);

    info->blockers[COLOR_WHITE] = compute_blockers(position, COLOR_WHITE);
    info->blockers[COLOR_BLACK] = compute_blockers(position, COLOR_BLACK);
    info->checkers              = compute_checkers(position, side_to_move);

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
        putchar('a' + (char)file_of_square(en_passant_square(position)));
        putchar('1' + (char)rank_of_square(en_passant_square(position)));
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
