#include "search.h"

#include <stdio.h>
#include <stdlib.h>

// material score in centipawns from the side to move's view
int evaluate(const Board *board)
{
    // indexed by the Piece enum, kings are 0 since both sides always have one
    static const int VALUES[] = {0, 100, 300, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0};
    int score = 0;
    for (int sq = 0; sq < 64; ++sq) score += VALUES[board->squares[sq]];
    return board->side_to_move == WHITE ? score : -score;
}

/*
searches within a provided chess position using a single thread.

this is the baseline for the performance of our parallel search implementations.

board: Starting position, treated as read-only.
depth: Maximum number of plies to search.

TODO: implement negamax with alpha-beta pruning (again i think carter had some implementation already)
*/
SearchResult search_sequential(const Board *board, int depth)
{
    (void)board;
    (void)depth;
    fputs("search_sequential is not implemented\n", stderr);
    abort();
}
