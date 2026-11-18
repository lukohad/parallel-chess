#include "search.h"

// wider than any score, used as the starting window
#define INF 1000000
// score for being checkmated, far above any material total
#define MATE 100000

// material score in centipawns from the side to move's view
int evaluate(const Board *board)
{
    // indexed by the Piece enum, kings are 0 since both sides always have one
    static const int VALUES[] = {0, 100, 300, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0};
    int score = 0;
    for (int sq = 0; sq < 64; ++sq) score += VALUES[board->squares[sq]];
    return board->side_to_move == WHITE ? score : -score;
}

// negamax with alpha-beta, same recursion as document/baseline/alphabeta.c but on real positions
// ply is the distance from the root, the root records the best move and every node bumps the count
static int negamax(const Board *board, int depth, int ply, int alpha, int beta, SearchResult *result)
{
    ++result->nodes;
    if (depth == 0) return evaluate(board);
    MoveList list;
    generate_legal_moves(board, &list);
    if (list.count == 0) return in_check(board) ? -MATE + ply : 0; // checkmate (sooner is worse), or stalemate
    for (int i = 0; i < list.count; ++i) {
        Board next;
        make_move(board, list.moves[i], &next);
        int score = -negamax(&next, depth - 1, ply + 1, -beta, -alpha, result);
        if (score >= beta) return beta; // cutoff, the opponent won't allow this line
        if (score > alpha) {
            alpha = score;
            if (ply == 0) {
                result->best_move = list.moves[i];
                result->has_best_move = true;
            }
        }
    }
    return alpha;
}

// single threaded search, the baseline every parallel version is compared against
SearchResult search_sequential(const Board *board, int depth)
{
    SearchResult result = {0};
    result.score = negamax(board, depth, 0, -INF, INF, &result);
    return result;
}
