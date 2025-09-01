#ifndef BITBOARD_H
#define BITBOARD_H


#include <stdint.h>
#include <stdio.h>



typedef uint64_t Bitboard;


void print_bitboard(FILE* stream, const Bitboard bitboard);



#endif /* BITBOARD_H */