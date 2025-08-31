from enum import Enum, auto


class Color(Enum):
    WHITE = 0
    BLACK = auto()

class Square(Enum):
    A1 = 0
    B1 = auto()
    C1 = auto()
    D1 = auto()
    E1 = auto()
    F1 = auto()
    G1 = auto()
    H1 = auto()
    A2 = auto()
    B2 = auto()
    C2 = auto()
    D2 = auto()
    E2 = auto()
    F2 = auto()
    G2 = auto()
    H2 = auto()
    A3 = auto()
    B3 = auto()
    C3 = auto()
    D3 = auto()
    E3 = auto()
    F3 = auto()
    G3 = auto()
    H3 = auto()
    A4 = auto()
    B4 = auto()
    C4 = auto()
    D4 = auto()
    E4 = auto()
    F4 = auto()
    G4 = auto()
    H4 = auto()
    A5 = auto()
    B5 = auto()
    C5 = auto()
    D5 = auto()
    E5 = auto()
    F5 = auto()
    G5 = auto()
    H5 = auto()
    A6 = auto()
    B6 = auto()
    C6 = auto()
    D6 = auto()
    E6 = auto()
    F6 = auto()
    G6 = auto()
    H6 = auto()
    A7 = auto()
    B7 = auto()
    C7 = auto()
    D7 = auto()
    E7 = auto()
    F7 = auto()
    G7 = auto()
    H7 = auto()
    A8 = auto()
    B8 = auto()
    C8 = auto()
    D8 = auto()
    E8 = auto()
    F8 = auto()
    G8 = auto()
    H8 = auto()

class File(Enum):
    FILE_A = 0
    FILE_B = auto()
    FILE_C = auto()
    FILE_D = auto()
    FILE_E = auto()
    FILE_F = auto()
    FILE_G = auto()
    FILE_H = auto()

class Rank(Enum):
    RANK_1 = 0
    RANK_2 = auto()
    RANK_3 = auto()
    RANK_4 = auto()
    RANK_5 = auto()
    RANK_6 = auto()
    RANK_7 = auto()
    RANK_8 = auto()

class Direction(Enum):
    NORTH = 8
    EAST = 1
    SOUTH = -NORTH
    WEST = -EAST


def print_bitboard(square, bitboard):
    for rank in range(7, -1, -1):
        print(f"{rank + 1}  ", end="")
        for file in range(8):
            mask = 1 << (8 * rank + file)
            if bitboard & mask == 0:
                print(" 0", end="")
            else:
                print(" 1", end="")
        print()

    print()
    print("    a b c d e f g h")
    print()

    print(f"Square:             {square}")
    print(f"Decimal value:      {bitboard}")
    print(f"Hexadecimal value:  {bitboard:#018X}")
    print()

def print_move_table_bitboards(move_table):
    for square, bitboard in enumerate(move_table):
        print_bitboard(square, bitboard)
        print()

def print_move_table(move_table):
    for bitboard in move_table:
        print(f"{bitboard:#018X}")


def generate_pawn_move_table(color):
    move_table = []

    for square in Square:
        bitboard = 0

        value = square.value
        rank = int(square.name[1]) - 1

        if rank != Rank.RANK_1.value and rank != Rank.RANK_8.value:
            if color == Color.WHITE:
                bitboard |= 1 << (value + Direction.NORTH.value)

                # Double move if pawn is on starting square.
                if rank == Rank.RANK_2.value:
                    bitboard |= 1 << (value + 2 * Direction.NORTH.value)
            elif color == Color.BLACK:
                bitboard |= 1 << (value + Direction.SOUTH.value)

                # Double move if pawn is on starting square.
                if rank == Rank.RANK_7.value:
                    bitboard |= 1 << (value + 2 * Direction.SOUTH.value)

        move_table.append(bitboard)

    return move_table

def generate_pawn_capture_table(color):
    move_table = []

    for square in Square:
        bitboard = 0

        value = square.value
        file = ord(square.name[0]) - ord('A')
        rank = int(square.name[1]) - 1

        if rank != Rank.RANK_1.value and rank != Rank.RANK_8.value:
            if color == Color.WHITE:
                # Northwest square.
                if file != File.FILE_A.value:
                    bitboard |= 1 << (value + Direction.NORTH.value + Direction.WEST.value)

                # Northeast square.
                if file != File.FILE_H.value:
                    bitboard |= 1 << (value + Direction.NORTH.value + Direction.EAST.value)
            elif color == Color.BLACK:
                # Southwest square.
                if file != File.FILE_A.value:
                    bitboard |= 1 << (value + Direction.SOUTH.value + Direction.WEST.value)

                # Southeast square.
                if file != File.FILE_H.value:
                    bitboard |= 1 << (value + Direction.SOUTH.value + Direction.EAST.value)

        move_table.append(bitboard)

    return  move_table

def generate_knight_move_table():
    move_table = []

    for square in Square:
        bitboard = 0

        value = square.value
        file = ord(square.name[0]) - ord('A')
        rank = int(square.name[1]) - 1

        # North-left square.
        if file >= File.FILE_B.value and rank <= Rank.RANK_6.value:
            bitboard |= 1 << (value + 2 * Direction.NORTH.value + Direction.WEST.value)

        # North-right square.
        if file <= File.FILE_G.value and rank <= Rank.RANK_6.value:
            bitboard |= 1 << (value + 2 * Direction.NORTH.value + Direction.EAST.value)

        # East-up square.
        if file <= File.FILE_F.value and rank <= Rank.RANK_7.value:
            bitboard |= 1 << (value + Direction.NORTH.value + 2 * Direction.EAST.value)

        # East-down square.
        if file <= File.FILE_F.value and rank >= Rank.RANK_2.value:
            bitboard |= 1 << (value + Direction.SOUTH.value + 2 * Direction.EAST.value)

        # South-right square.
        if file <= File.FILE_G.value and rank >= Rank.RANK_3.value:
            bitboard |= 1 << (value + 2 * Direction.SOUTH.value + Direction.EAST.value)

        # South-left square.
        if file >= File.FILE_B.value and rank >= Rank.RANK_3.value:
            bitboard |= 1 << (value + 2 * Direction.SOUTH.value + Direction.WEST.value)

        # West-down square.
        if file >= File.FILE_C.value and rank >= Rank.RANK_2.value:
            bitboard |= 1 << (value + Direction.SOUTH.value + 2 * Direction.WEST.value)

        # West-up square.
        if file >= File.FILE_C.value and rank <= Rank.RANK_7.value:
            bitboard |= 1 << (value + Direction.NORTH.value + 2 * Direction.WEST.value)

        move_table.append(bitboard)

    return move_table

def generate_bishop_move_table():
    move_table = []

    for square in Square:
        bitboard = 0

        file = ord(square.name[0]) - ord('A')
        rank = int(square.name[1]) - 1

        # Northeast diagonal squares.
        current_file = file + 1
        current_rank = rank + 1
        while current_file <= File.FILE_H.value and current_rank <= Rank.RANK_8.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_file += 1
            current_rank += 1

        # Southeast diagonal squares.
        current_file = file + 1
        current_rank = rank - 1
        while current_file <= File.FILE_H.value and current_rank >= Rank.RANK_1.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_file += 1
            current_rank -= 1

        # Southwest diagonal squares.
        current_file = file - 1
        current_rank = rank - 1
        while current_file >= File.FILE_A.value and current_rank >= Rank.RANK_1.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_file -= 1
            current_rank -= 1

        # Northwest diagonal squares.
        current_file = file - 1
        current_rank = rank + 1
        while current_file >= File.FILE_A.value and current_rank <= Rank.RANK_8.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_file -= 1
            current_rank += 1

        move_table.append(bitboard)

    return move_table

def generate_rook_move_table():
    move_table = []

    for square in Square:
        bitboard = 0

        file = ord(square.name[0]) - ord('A')
        rank = int(square.name[1]) - 1

        # North squares.
        current_file = file
        current_rank = rank + 1
        while current_rank <= Rank.RANK_8.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_rank += 1

        # East squares.
        current_file = file + 1
        current_rank = rank
        while current_file <= File.FILE_H.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_file += 1

        # South squares.
        current_file = file
        current_rank = rank - 1
        while current_rank >= Rank.RANK_1.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_rank -= 1

        # West squares.
        current_file = file - 1
        current_rank = rank
        while current_file >= File.FILE_A.value:
            bitboard |= 1 << (Direction.EAST.value * current_file + Direction.NORTH.value * current_rank)
            current_file -= 1

        move_table.append(bitboard)

    return move_table

def generate_king_move_table():
    move_table = []

    for square in Square:
        bitboard = 0

        value = square.value
        file = ord(square.name[0]) - ord('A')
        rank = int(square.name[1]) - 1

        # North squares.
        if rank <= Rank.RANK_7.value:
            bitboard |= 1 << (value + Direction.NORTH.value)

            # Northeast square.
            if file <= File.FILE_G.value:
                bitboard |= 1 << (value + Direction.NORTH.value + Direction.EAST.value)
            
            # Northwest square.
            if file >= File.FILE_B.value:
                bitboard |= 1 << (value + Direction.NORTH.value + Direction.WEST.value)
        
        # East square.
        if file <= File.FILE_G.value:
            bitboard |= 1 << (value + Direction.EAST.value)
        
        # South squares.
        if rank >= Rank.RANK_2.value:
            bitboard |= 1 << (value + Direction.SOUTH.value)

            # Southeast square.
            if file <= File.FILE_G.value:
                bitboard |= 1 << (value + Direction.SOUTH.value + Direction.EAST.value)

            # Southwest square.
            if file >= File.FILE_B.value:
                bitboard |= 1 << (value + Direction.SOUTH.value + Direction.WEST.value)

        # West square.
        if file >= File.FILE_B.value:
            bitboard |= 1 << (value +Direction.WEST.value)

        move_table.append(bitboard)

    return move_table


def main():
    white_pawn_move_table = generate_pawn_move_table(Color.WHITE)
    # print_move_table_bitboards(white_pawn_move_table)
    print("White pawn move table:")
    print_move_table(white_pawn_move_table)
    print()

    black_pawn_move_table = generate_pawn_move_table(Color.BLACK)
    # print_move_table_bitboards(black_pawn_move_table)
    print("Black pawn move table:")
    print_move_table(black_pawn_move_table)
    print()

    white_pawn_capture_table = generate_pawn_capture_table(Color.WHITE)
    # print_move_table_bitboards(white_pawn_capture_table)
    print("White pawn capture table:")
    print_move_table(white_pawn_capture_table)
    print()

    black_pawn_capture_table = generate_pawn_capture_table(Color.BLACK)
    # print_move_table_bitboards(black_pawn_capture_table)
    print("Black pawn capture table:")
    print_move_table(black_pawn_capture_table)
    print()

    knight_move_table = generate_knight_move_table()
    # print_move_table_bitboards(knight_move_table)
    print("Knight move table:")
    print_move_table(knight_move_table)
    print()

    bishop_move_table = generate_bishop_move_table()
    # print_move_table_bitboards(bishop_move_table)
    print("Bishop move table:")
    print_move_table(bishop_move_table)
    print()

    rook_move_table = generate_rook_move_table()
    # print_move_table_bitboards(rook_move_table)
    print("Rook move table:")
    print_move_table(rook_move_table)
    print()

    # There is no need to compute a separate move table for queen moves as it is just the bitwise or of the bishop and rook moves.

    king_move_table = generate_king_move_table()
    # print_move_table_bitboards(king_move_table)
    print("King move table:")
    print_move_table(king_move_table)
    print()


if __name__ == "__main__":
    main()

