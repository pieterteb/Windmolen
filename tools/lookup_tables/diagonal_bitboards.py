import numpy as np
import os

import bitboard_utils as bbu


def generate_diagonal_bitboards():
    diagonal_bitboards = np.zeros(64, dtype=np.uint64)
    antidiagonal_bitboards = np.zeros(64, dtype=np.uint64)

    for square in range(64):
        rank = bbu.rank_from_square(square)
        file = bbu.file_from_square(square)

        diagonal_bitboard = 0
        antidiagonal_bitboard = 0

        for r in range(8):
            for f in range(8):
                s = f + r * 8
                if (r - f) == (rank - file):  # same diagonal
                    diagonal_bitboard |= bbu.square_bitboard(s)
                if (r + f) == (rank + file):  # same anti-diagonal
                    antidiagonal_bitboard |= bbu.square_bitboard(s)

        diagonal_bitboards[square] = diagonal_bitboard
        antidiagonal_bitboards[square] = antidiagonal_bitboard

    return diagonal_bitboards, antidiagonal_bitboards

def write_bitboards_file():
    diagonal_bitboards, antidiagonal_bitboards = generate_diagonal_bitboards()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "diagonal_bitboards.txt")

    with open(output_file, "w") as f:
        def write_bitboard_array(name, array):
            f.write(f"const Bitboard {name}[SQUARE_COUNT] = {{\n")
            for rank in range(8):
                f.write("    ")
                for file in range(8):
                    square = file + rank * 8
                    f.write(f"0x{array[square]:016x}")
                    if file != 7:
                        f.write(", ")
                    elif rank != 7:
                        f.write(",")
                if rank != 7:
                    f.write("\n")
            f.write("\n};")

        write_bitboard_array("diagonal_bitboards", diagonal_bitboards)
        f.write("\n")
        write_bitboard_array("antidiagonal_bitboards", antidiagonal_bitboards)

def main():
    write_bitboards_file()

if __name__ == "__main__":
    main()
