import numpy as np
import os

import bitboard_utils as bbu
import piece_base_attacks_table as pbat


def generate_line_bitboards(piece_base_attacks_table):
    line_bitboards = np.zeros((64, 64), dtype=np.uint64)
    for square1 in range(64):
        square1_bitboard = bbu.square_bitboard(square1)
        for square2 in range(64):
            square2_bitboard = bbu.square_bitboard(square2)

            if (piece_base_attacks_table[2, square1] & square2_bitboard) != 0:
                line_bitboards[square1][square2] = (piece_base_attacks_table[2, square1] & piece_base_attacks_table[2, square2]) | square1_bitboard | square2_bitboard
            elif (piece_base_attacks_table[3, square1] & square2_bitboard) != 0:
                line_bitboards[square1][square2] = (piece_base_attacks_table[3, square1] & piece_base_attacks_table[3, square2]) | square1_bitboard | square2_bitboard

    return line_bitboards

def line_bitboards_file():
    piece_base_attacks_table = pbat.generate_piece_base_attacks_table()
    line_bitboards = generate_line_bitboards(piece_base_attacks_table)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "line_bitboards.txt")

    with open(output_file, "w") as f:
        f.write("const Bitboard line_bitboards[SQUARE_COUNT][SQUARE_COUNT] = {\n")
        
        for square1 in range(64):
            f.write("    {")
            for rank in range(8):
                if rank != 0:
                    f.write("     ")
                for file in range(8):
                    square2 = file + rank * 8
                    f.write(f"0x{line_bitboards[square1, square2]:016x}")
                    if file != 7:
                        f.write(", ")
                    elif rank != 7:
                        f.write(",")
                if rank != 7:
                    f.write("\n")
            f.write("}")
            if square1 != 63:
                f.write(",\n\n")

        f.write("\n};")

def main():
    line_bitboards_file()

if __name__ == "__main__":
    main()
