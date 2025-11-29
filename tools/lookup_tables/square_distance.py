import numpy as np
import os

import bitboard_utils as bbu


def generate_square_distances():
    square_distances = np.zeros((64, 64), dtype=np.uint8)
    for square1 in range(64):
        for square2 in range(64):
            square_distances[square1, square2] = np.max([np.abs(bbu.file_from_square(square1) - bbu.file_from_square(square2)), np.abs(bbu.rank_from_square(square1) - bbu.rank_from_square(square2))])

    return square_distances

def square_distances_file():
    square_distances = generate_square_distances()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, "square_distances.txt")

    with open(output_file, "w") as f:
        f.write("const uint8_t square_distances[SQUARE_COUNT][SQUARE_COUNT] = {\n")
        
        for square1 in range(64):
            f.write("    {")
            for rank in range(8):
                if rank != 0:
                    f.write("     ")
                for file in range(8):
                    square2 = file + rank * 8
                    f.write(f"{square_distances[square1, square2]:02}")
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
    square_distances_file()

if __name__ == "__main__":
    main()
