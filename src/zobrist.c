#include "zobrist.h"

#include <assert.h>
#include <stddef.h>

#include "util.h"



ZobristKey piece_zobrist_keys[PIECE_COUNT][SQUARE_COUNT];
ZobristKey castle_zobrist_keys[CASTLE_COUNT];
ZobristKey en_passant_zobrist_keys[FILE_COUNT];
ZobristKey side_to_move_zobrist_key;


void initialize_zobrist_keys() {
    static_assert(sizeof(ZobristKey) == 8U, "ZobristKey type must be a 64-bit integer.");

    seed_rand64(15146693);

    for (enum Piece piece = PIECE_WHITE_PAWN; piece < PIECE_COUNT; ++piece)
        for (enum Square square = SQUARE_A1; square < SQUARE_COUNT; ++square)
            piece_zobrist_keys[piece][square] = rand64();

    for (enum CastlingRights castling_rights = CASTLE_NONE; castling_rights < CASTLE_COUNT; ++castling_rights)
        castle_zobrist_keys[castling_rights] = rand64();

    for (enum File file = FILE_A; file < FILE_COUNT; ++file)
        en_passant_zobrist_keys[file] = rand64();

    side_to_move_zobrist_key = rand64();
}
