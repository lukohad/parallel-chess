#include "movegen.h"

#include <stdio.h>
#include <stdlib.h>

/*
generate all legal moves for the player whose turn it is.

board: current position, treated as read-only.
list:  destination for generated moves.

TODO: implement move generation and king-safety checks.
    king safety checks like if a player is in check 
    or if a move would leave the king in check like pins
*/
void generate_legal_moves(const Board *board, MoveList *list)
{
    (void)board;
    (void)list;
    fputs("generate_legal_moves is not implemented\n", stderr);
    abort();
}

/*
Applies a move to a copy of the current position.

board: original position, treated as read-only.
move:  move to apply.
next:  receives the resulting board state.

the original position should remain unchanged.
TODO: implement piece movement and board-state updates.
*/
void make_move(const Board *board, Move move, Board *next)
{
    (void)board;
    (void)move;
    (void)next;
    fputs("make_move is not implemented\n", stderr);
    abort();
}
