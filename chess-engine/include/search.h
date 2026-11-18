#ifndef SEARCH_H
#define SEARCH_H

#include <stdbool.h>
#include "movegen.h"

/*
best_move:     move selected by the search algorithm.
score:         evaluation relative to the side to move at the root, value in centipawns
                    positive means favorable for the side to move at the root. 
                    negamax evals must consistently follow the side-to-move perspective at each node
nodes:         total number of search nodes visited.
                    depth is measured in plies (half-moves)
has_best_move: indicates whether best_move contains a valid move.

a search may produce an evaluation without returning a move,
such as when the current position is checkmate or stalemate.

the node count will be used for metrics
*/
typedef struct {
    Move best_move;
    int score;
    uint64_t nodes;
    bool has_best_move;
} SearchResult;

/*
searches a chess position using a single thread.

board: Starting position, treated as read-only.
depth: Maximum search depth measured in plies (halfmoves).

returns a SearchResult containing the selected move,
evaluation score, and number of nodes searched.

TODO: implement negamax with alpha-beta pruning. (i think carter had some implementation already)
*/
SearchResult search_sequential(const Board *board, int depth);

//material score in centipawns from the side to move's view
int evaluate(const Board *board);

#endif
