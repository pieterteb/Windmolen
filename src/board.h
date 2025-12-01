#ifndef WINDMOLEN_BOARD_H_
#define WINDMOLEN_BOARD_H_


#include <stdint.h>
#include <stdlib.h>

#include "piece.h"
#include "util.h"



// We describe the board by assigning an index from 0 through 63 to each square of the board. a1 has index 0 and h8 has
// index 63 such that indices increase by 1 if moving east along a row and by 8 if moving north along a file.
// clang-format off
enum Square : uint8_t {
    SQUARE_A1, SQUARE_B1, SQUARE_C1, SQUARE_D1, SQUARE_E1, SQUARE_F1, SQUARE_G1, SQUARE_H1,
    SQUARE_A2, SQUARE_B2, SQUARE_C2, SQUARE_D2, SQUARE_E2, SQUARE_F2, SQUARE_G2, SQUARE_H2,
    SQUARE_A3, SQUARE_B3, SQUARE_C3, SQUARE_D3, SQUARE_E3, SQUARE_F3, SQUARE_G3, SQUARE_H3,
    SQUARE_A4, SQUARE_B4, SQUARE_C4, SQUARE_D4, SQUARE_E4, SQUARE_F4, SQUARE_G4, SQUARE_H4,
    SQUARE_A5, SQUARE_B5, SQUARE_C5, SQUARE_D5, SQUARE_E5, SQUARE_F5, SQUARE_G5, SQUARE_H5,
    SQUARE_A6, SQUARE_B6, SQUARE_C6, SQUARE_D6, SQUARE_E6, SQUARE_F6, SQUARE_G6, SQUARE_H6,
    SQUARE_A7, SQUARE_B7, SQUARE_C7, SQUARE_D7, SQUARE_E7, SQUARE_F7, SQUARE_G7, SQUARE_H7,
    SQUARE_A8, SQUARE_B8, SQUARE_C8, SQUARE_D8, SQUARE_E8, SQUARE_F8, SQUARE_G8, SQUARE_H8,

    SQUARE_COUNT,

    SQUARE_NONE = SQUARE_COUNT
};
// clang-format on
static_assert(SQUARE_COUNT == 64);

// Returns whether `square` is valid. This is the case if `square` lies in the range [SQUARE_A1, SQUARE_H8].
static INLINE bool is_valid_square(const enum Square square) {
    return square < SQUARE_COUNT;
}


// Returns the starting square of the king of `color`, assuming `color` is valid.
static INLINE enum Square king_start_square(const enum Color color) {
    assert(is_valid_color(color));
    static_assert(SQUARE_E8 - SQUARE_E1 == 56);

    // We have SQUARE_E8 - SQUARE_E1 == 56 == 7 << 3. So this evaluates to SQUARE_E1 if color == COLOR_WHITE and
    // SQUARE_E8 if color == COLOR_BLACK. This is faster than a regular ternary: https://godbolt.org/z/xEePT961G
    return SQUARE_E1 + (enum Square)((color * 7) << 3);
}


enum File : uint8_t {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,

    FILE_COUNT
};
static_assert(FILE_COUNT == 8);

// Returns whether `file` is valid. This is the case if `file` lies in the range [FILE_A, FILE_H].
static INLINE bool is_valid_file(const enum File file) {
    return file < FILE_COUNT;
}

// Returns the file that `square` lies on, assuming `square` is valid.
static INLINE enum File file_of_square(const enum Square square) {
    assert(is_valid_square(square));

    return square & 7;  // Fast modulo 8.
}

// Returns the file corresponding to `c` (lowercase), assuming `c` is one of a,b,...,h.
static INLINE enum File char_to_file(const char c) {
    assert(c >= 'a' && c <= 'h');

    return (enum File)(c - 'a');
}


enum Rank : uint8_t {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,

    RANK_COUNT
};
static_assert(RANK_COUNT == 8);

// Returns whether `rank` is valid. This is the case if `rank` lies in the range [RANK_1, RANK_8].
static INLINE bool is_valid_rank(const enum Rank rank) {
    return rank < RANK_COUNT;
}

// Returns the rank that `square` lies on, assuming `square` is valid.
static INLINE enum Rank rank_of_square(const enum Square square) {
    assert(is_valid_square(square));

    return square >> 3;  // Fast division by 8.
}

// Returns the rank corresponding to `c`, assuming `c` is one of 1,2,...,8.
static INLINE enum Rank char_to_rank(const char c) {
    assert(c >= '1' && c <= '8');

    return (enum Rank)(c - '1');
}


/* The distance between x and y is defined as the number of king moves required to go from x to y. */
extern const uint8_t square_distances[SQUARE_COUNT][SQUARE_COUNT];

// Returns the distance between `square1` and `square2`, assuming `square1` and `square2` are valid.
static INLINE uint8_t distance(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return square_distances[square1][square2];  // Precomputed by ../tools/lookup_table/square_distances.py.
}

// Returns the distance between the files that `square1` and `square2` lie on.
static INLINE uint8_t file_distance(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return (uint8_t)abs(file_of_square(square1) - file_of_square(square2));
}

// Returns the distance between the ranks that `square1` and `square2` lie on.
static INLINE uint8_t rank_distance(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return (uint8_t)abs(rank_of_square(square1) - rank_of_square(square2));
}


enum Direction : int8_t {
    DIRECTION_NORTH = 8,
    DIRECTION_EAST  = 1,
    DIRECTION_SOUTH = -DIRECTION_NORTH,
    DIRECTION_WEST  = -DIRECTION_EAST,

    DIRECTION_NORTHEAST = DIRECTION_NORTH + DIRECTION_EAST,
    DIRECTION_SOUTHEAST = DIRECTION_SOUTH + DIRECTION_EAST,
    DIRECTION_SOUTHWEST = DIRECTION_SOUTH + DIRECTION_WEST,
    DIRECTION_NORTHWEST = DIRECTION_NORTH + DIRECTION_WEST,

    DIRECTION_NORTH2 = 2 * DIRECTION_NORTH,
    DIRECTION_SOUTH2 = 2 * DIRECTION_SOUTH
};

// Returns the square described by `file` and `rank`, assuming `file` and `rank` are valid.
static INLINE enum Square square_from_coordinates(enum File file, enum Rank rank) {
    assert(is_valid_file(file));
    assert(is_valid_rank(rank));

    // Should compile to a single LEA instruction on modern CPUs.
    return (enum Square)((enum Direction)file * DIRECTION_EAST + (enum Direction)rank * DIRECTION_NORTH);
}


// Returns the square `direction` away from `square`, assuming `square` is valid.
static INLINE enum Square square_step(const enum Square square, const enum Direction direction) {
    assert(is_valid_square(square));

    return square + (enum Square)direction;
}

// Returns the square north of `square`, assuming `square` is valid and not on the 8th rank.
static INLINE enum Square square_north(const enum Square square) {
    assert(is_valid_square(square));
    assert(rank_of_square(square) != RANK_8);

    return square + (enum Square)DIRECTION_NORTH;
}

// Returns the 2 steps north of `square`, assuming `square` is valid and not on the 7th or 8th rank.
static INLINE enum Square square_2north(const enum Square square) {
    assert(is_valid_square(square));
    assert(rank_of_square(square) < RANK_7);

    return square + (enum Square)DIRECTION_NORTH2;
}

// Returns the square south of `square`, assuming `square` is valid and not on the 1st rank.
static INLINE enum Square square_south(const enum Square square) {
    assert(is_valid_square(square));
    assert(rank_of_square(square) != RANK_1);

    return square + (enum Square)DIRECTION_SOUTH;
}

// Returns the 2 steps south of `square`, assuming `square` is valid and not on the 1st or 2nd rank.
static INLINE enum Square square_2south(const enum Square square) {
    assert(is_valid_square(square));
    assert(rank_of_square(square) > RANK_2);

    return square + (enum Square)DIRECTION_SOUTH2;
}

// Returns the square east of `square`, assuming `square` is valid and not on the h-file.
static INLINE enum Square square_east(const enum Square square) {
    assert(is_valid_square(square));
    assert(file_of_square(square) != FILE_H);

    return square + (enum Square)DIRECTION_EAST;
}

// Returns the square west of `square`, assuming `square` is valid and not on the a-file.
static INLINE enum Square square_west(const enum Square square) {
    assert(is_valid_square(square));
    assert(file_of_square(square) != FILE_A);

    return square + (enum Square)DIRECTION_WEST;
}

// Returns the square northeast of `square`, assuming `square` is valid and not on the h-file or 8th rank.
static INLINE enum Square square_northeast(const enum Square square) {
    assert(is_valid_square(square));
    assert(file_of_square(square) != FILE_H && rank_of_square(square) != RANK_8);

    return square + (enum Square)DIRECTION_NORTHEAST;
}

// Returns the square southeast of `square`, assuming `square` is valid and not on the h-file or 1st rank.
static INLINE enum Square square_southeast(const enum Square square) {
    assert(is_valid_square(square));
    assert(file_of_square(square) != FILE_H && rank_of_square(square) != RANK_1);

    return square + (enum Square)DIRECTION_SOUTHEAST;
}

// Returns the square southwest of `square`, assuming `square` is valid and not on the a-file or 1st rank.
static INLINE enum Square square_southwest(const enum Square square) {
    assert(is_valid_square(square));
    assert(file_of_square(square) != FILE_A && rank_of_square(square) != RANK_1);

    return square + (enum Square)DIRECTION_SOUTHWEST;
}

// Returns the square northwest of `square`, assuming `square` is valid and not on the a-file or 8th rank.
static INLINE enum Square square_northwest(const enum Square square) {
    assert(is_valid_square(square));
    assert(file_of_square(square) != FILE_A && rank_of_square(square) != RANK_8);

    return square + (enum Square)DIRECTION_NORTHWEST;
}



#endif /* #ifndef WINDMOLEN_BOARD_H_ */
