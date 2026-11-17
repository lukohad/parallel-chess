#include "board.h"

#include <stdio.h>

//initialize the board to the starting position
void board_init(Board *board)
{
    *board = (Board) {
        .squares = {
            WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,
            WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
            BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
            BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK
        },
        .side_to_move = WHITE,
        .castling_rights = CASTLE_WHITE_KINGSIDE | CASTLE_WHITE_QUEENSIDE |
                           CASTLE_BLACK_KINGSIDE | CASTLE_BLACK_QUEENSIDE,
        .en_passant = -1,
        .halfmove_clock = 0,
        .fullmove_number = 1
    };
}

void board_print(const Board *board)
{
    //lookup table for piece symbols when printing the board
    //array index corresponds to the Piece enum value
    //capital letters for white pieces, lowercase for black pieces, and '.' for empty squares
    static const char symbols[] = ".PNBRQKpnbrqk";

    //print the board from rank 8 to rank 1
    for (int rank = 7; rank >= 0; --rank) {
        printf("%d  ", rank + 1);
        //print each file in the rank
        for (int file = 0; file < 8; ++file) {
            printf("%c ", symbols[board->squares[rank * 8 + file]]);
        }
        putchar('\n');
    }
    //print the file labels and side to move
    printf("\n   a b c d e f g h\n");
    //print the side to move
    printf("Side to move: %s\n", board->side_to_move == WHITE ? "White" : "Black");
}
