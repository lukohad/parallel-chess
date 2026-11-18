#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"


/*
bit flags identifying special move properties for the move struct below
each flag occupies a separate bit, allowing combinations

MOVE_NONE:        0x00 = 00000000 (ordinary move)
MOVE_CAPTURE:     0x01 = 00000001 (captures a piece)
MOVE_DOUBLE_PAWN: 0x02 = 00000010 (pawn advances two squares)
MOVE_EN_PASSANT:  0x04 = 00000100 (en passant capture)
MOVE_CASTLING:    0x08 = 00001000 (castling move)
MOVE_PROMOTION:   0x10 = 00010000 (pawn promotion)

some moves require multiple flags.
for example, en passant uses MOVE_CAPTURE | MOVE_EN_PASSANT.
*/
typedef enum {
    MOVE_NONE         = 0,
    MOVE_CAPTURE      = 1 << 0,
    MOVE_DOUBLE_PAWN  = 1 << 1,
    MOVE_EN_PASSANT   = 1 << 2,
    MOVE_CASTLING     = 1 << 3,
    MOVE_PROMOTION    = 1 << 4
} MoveFlag;

//move representation
/*
from: the square the piece is moving from (0-63)
to: the square the piece is moving to (0-63)
    e2 to e4 would be from = 12 and to = 28
promotion: the piece type a pawn would to promote
    pretty much always a queen, but can be a knight, rook, or bishop
    most of the time it will have value EMPTY
flags: a bitmask for special move types: captures, castling, en passant, promotion, two square pawn moves, etc.
    ref for flags: https://chessprogramming.org/Encoding_Moves

*/
typedef struct {
    uint8_t from;
    uint8_t to;
    Piece promotion;
    uint8_t flags;
} Move;

#define MAX_MOVES 256

/*
Applies a move to a copy of the current position.

board: Original position, treated as read-only.
move:  Move to apply.
next:  Receives the resulting board state.

The original position should remain unchanged 
*/
typedef struct {
    Move moves[MAX_MOVES];
    int count;
} MoveList;

//function prototypes
void generate_legal_moves(const Board *board, MoveList *list);
void make_move(const Board *board, Move move, Board *next);
//true if the side to move is in check
bool in_check(const Board *board);

#endif
