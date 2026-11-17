#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

//piece definitions

/*
EMPTY	        0
WHITE_PAWN	    1
WHITE_KNIGHT	2
WHITE_BISHOP	3
WHITE_ROOK	    4
WHITE_QUEEN	    5
WHITE_KING	    6
BLACK_PAWN	    7
BLACK_KNIGHT	8
BLACK_BISHOP	9
BLACK_ROOK	    10
BLACK_QUEEN	    11
BLACK_KING	    12

Piece piece = WHITE_KNIGHT is same as Piece piece = 2;
*/
typedef enum {
    EMPTY,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP,
    WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP,
    BLACK_ROOK, BLACK_QUEEN, BLACK_KING
} Piece;


//more of a flag for players
typedef enum {
    WHITE,
    BLACK
} Color;

/* castling rights
hexadecimals for a bitmask

0000 or 0x00 means no castling rights
0001 or 0x01 means white can castle kingside
0010 or 0x02 means white can castle queenside
0100 or 0x04 means black can castle kingside
1000 or 0x08 means black can castle queenside

1100 or 0x0C means white can castle both sides
and so on
*/
#define CASTLE_WHITE_KINGSIDE  0x01
#define CASTLE_WHITE_QUEENSIDE 0x02
#define CASTLE_BLACK_KINGSIDE  0x04
#define CASTLE_BLACK_QUEENSIDE 0x08


/*
board is a 2d array of 8x8 squares
    0 is a1
    1 is b1
    7 is h1
    8 is a2
    63 is h8
    
color is the flag for which side is to move
castling_rights is a bitmask for which sides can castle
en_passant is a number for which square the pawn will land after an en passant capture
halfmove_clock is the number of halfmoves since the last capture or pawn advance
    a halfmove is a move by one player
fullmove_number is the number of full moves in the game
    a full move is a pair of moves, one by white and one by black
*/
typedef struct {
    Piece squares[64]; // a1 = 0, h8 = 63 
    Color side_to_move;
    uint8_t castling_rights;
    int en_passant; // target square, or -1 when unavailable
    int halfmove_clock;
    int fullmove_number;
} Board;

//function prototypes
//initialize the board to the starting position
void board_init(Board *board);
//print the board to the console
void board_print(const Board *board);

/*
initial board state:
side_to_move = WHITE
castling_rights = 0x0F (all castling rights available)
en_passant = -1 (no en passant available)
halfmove_clock = 0 (no halfmoves since last capture or pawn advance)
fullmove_number = 1 (first full move of the game)
*/


#endif
