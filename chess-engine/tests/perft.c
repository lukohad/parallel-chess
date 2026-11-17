#include "board.h"

#include <stdio.h>
#include <stdlib.h>

/*
perft (performance test) gets the number of legal move
sequences reachable from a position at a given depth.

its for verification of move generation
by comparing the resulting node count against known values.

it does not evaluate positions or prune branches 
it explores every legal move at each depth.

board: Starting position, treated as read-only.
depth: Number of plies (halfmoves) to explore.

returns the number of leaf nodes reached at the given depth.

TODO: implement recursive perft using generate_legal_moves()
and make_move().
*/
uint64_t perft(const Board *board, int depth);

uint64_t perft(const Board *board, int depth)
{
    (void)board;
    (void)depth;
    fputs("perft is not implemented\n", stderr);
    abort();
}
