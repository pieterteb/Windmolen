import numpy as np


def print_bitboard(bitboard):
    print("+---+---+---+---+---+---+---+---+")
    for rank in range(7, -1, -1):
        for file in range(8):
            print("| X " if (bitboard & (1 << (file + 8 * rank))) != 0 else "|   ", end="")
        print(f"| {rank + 1}\n+---+---+---+---+---+---+---+---+")
    print("  a   b   c   d   e   f   g   h\nHex: 0x{bitboard:016x}".format(bitboard=bitboard))

def file_from_square(square):
    return square & 7

def rank_from_square(square):
    return square >> 3

def step_safe(source, step):
    destination = source + step
    if destination < 0 or destination >= 64:
        return np.uint64(0)
    
    file_difference = abs((destination & 7) - (source & 7))
    rank_difference = abs((destination >> 3) - (source >> 3))

    return np.uint64(1 << destination) if rank_difference <= 2 and file_difference <= 2 else np.uint64(0)

def square_bitboard(square):
    return np.uint64(1 << square)

def shift_bitboard(bitboard, direction):
    FILE_A = np.uint64(0x0101010101010101)
    FILE_H = np.uint64(0x8080808080808080)

    bitboard = np.uint64(bitboard)

    if direction == 8:
        return bitboard << 8
    elif direction == 16:
        return bitboard << 16
    elif direction == -8:
        return bitboard >> 8
    elif direction == -16:
        return bitboard >> 16
    elif direction == 1:
        return (bitboard & ~FILE_H) << 1
    elif direction == -1:
        return (bitboard & ~FILE_A) >> 1
    elif direction == 9:
        return (bitboard & ~FILE_H) << 9
    elif direction == -7:
        return (bitboard & ~FILE_H) >> 7
    elif direction == -9:
        return (bitboard & ~FILE_A) >> 9
    elif direction == 7:
        return (bitboard & ~FILE_A) << 7
    else:
        return np.uint64(0)
