import numpy as np
import os

import bitboard_utils as bbu


def generate_piece_base_attacks_table():
    piece_base_attacks_table = np.zeros((7, 64), dtype=np.uint64)
    for square in range(64):
        bitboard = bbu.square_bitboard(square)

        # White pawns.
        piece_base_attacks_table[0, square] = bbu.shift_bitboard(bitboard, 7) | bbu.shift_bitboard(bitboard, 9)

        # Black pawns.
        piece_base_attacks_table[6, square] = bbu.shift_bitboard(bitboard, -7) | bbu.shift_bitboard(bitboard, -9)

        # Knights.
        for step in np.array([17, 10, -6, -15, -17, -10, 6, 15]):
            piece_base_attacks_table[1, square] |= bbu.step_safe(square, step)

        # Kings.
        for step in np.array([8, 9, 1, -7, -8, -9, -1, 7]):
            piece_base_attacks_table[5, square] |= bbu.step_safe(square, step)

        # Bishops.
        for direction in np.array([9, -7, -9, 7]):
            temp = bbu.shift_bitboard(bitboard, direction)
            while temp != 0:
                piece_base_attacks_table[2, square] |= temp
                temp = bbu.shift_bitboard(temp, direction)

        # Rooks.
        for direction in np.array([8, 1, -8, -1]):
            temp = bbu.shift_bitboard(bitboard, direction)
            while temp != 0:
                piece_base_attacks_table[3, square] |= temp
                temp = bbu.shift_bitboard(temp, direction)

        # Queens.
        piece_base_attacks_table[4, square] = piece_base_attacks_table[2, square] | piece_base_attacks_table[3, square]
        
    return piece_base_attacks_table

def piece_base_attacks_table_file():
    piece_type_names = [
        "PIECE_TYPE_WHITE_PAWN",
        "PIECE_TYPE_KNIGHT",
        "PIECE_TYPE_BISHOP",
        "PIECE_TYPE_ROOK",
        "PIECE_TYPE_QUEEN",
        "PIECE_TYPE_KING",
        "PIECE_TYPE_BLACK_PAWN"
    ]
    max_name_length = max(len(name) for name in piece_type_names)
    piece_base_attacks_table = generate_piece_base_attacks_table()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "piece_base_attacks_table.txt")

    with open(output_file, "w") as f:
        f.write(f"const Bitboard piece_base_attacks_table[PIECE_TYPE_COUNT][SQUARE_COUNT] = {{\n")
        
        for idx, name in enumerate(piece_type_names):
            padding = " " * (max_name_length - len(name))
            f.write(f"    [{name}]{padding} = {{\n")
            for rank in range(8):
                f.write(f"        ")
                for file in range(8):
                    square = file + rank * 8
                    f.write(f"0x{piece_base_attacks_table[idx, square]:016x}")
                    if file != 7:
                        f.write(", ")
                    elif rank != 7:
                        f.write(",")
                f.write("\n")
            f.write("    }")
            if idx != 6:
                f.write(",\n\n")
        
        f.write("\n};")

def main():
    piece_base_attacks_table_file()

if __name__ == "__main__":
    main()
