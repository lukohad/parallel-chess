#include "search.h"

#include <stdio.h>
#include <stdlib.h>

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
