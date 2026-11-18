#include "search.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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

// put captures first so alpha-beta finds cutoffs sooner
static void order_moves(MoveList *list)
{
    int front = 0;
    for (int i = 0; i < list->count; ++i) {
        if (!(list->moves[i].flags & MOVE_CAPTURE)) continue;
        Move capture = list->moves[i];
        list->moves[i] = list->moves[front];
        list->moves[front++] = capture;
    }
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
    order_moves(&list);
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

// one thread's dealt root moves and what it found
typedef struct {
    const Board *board;
    MoveList moves;
    int depth;
    SearchResult result;
} StaticTask;

// search this thread's root moves with a private alpha, nothing here is shared with other threads
// repeats negamax's root best-move tracking, merging them would complicate sequential's no-moves and depth 0 cases
static void *static_worker(void *arg)
{
    StaticTask *task = arg;
    int alpha = -INF;
    for (int i = 0; i < task->moves.count; ++i) {
        Move move = task->moves.moves[i];
        Board next;
        make_move(task->board, move, &next);
        int score = -negamax(&next, task->depth - 1, 1, -INF, -alpha, &task->result);
        if (score > alpha) {
            alpha = score;
            task->result.best_move = move;
            task->result.has_best_move = true;
        }
    }
    task->result.score = alpha;
    return NULL;
}

// print what failed and stop, a thread that can't start or finish leaves no valid result
static void die(const char *message)
{
    fputs(message, stderr);
    abort();
}

// static root split, root moves are dealt round-robin to threads and the best result wins after the join
// on a tie the lowest numbered thread wins, so the best move can differ from sequential but the score can't
SearchResult search_static(const Board *board, int depth, int threads)
{
    MoveList moves;
    generate_legal_moves(board, &moves);
    if (depth < 1 || moves.count == 0) return search_sequential(board, depth); // nothing to split
    order_moves(&moves);

    StaticTask tasks[MAX_THREADS];
    for (int t = 0; t < threads; ++t) tasks[t] = (StaticTask){.board = board, .depth = depth};
    for (int i = 0; i < moves.count; ++i) { // move i goes to thread i % threads
        MoveList *hand = &tasks[i % threads].moves;
        hand->moves[hand->count++] = moves.moves[i];
    }

    pthread_t ids[MAX_THREADS];
    for (int t = 0; t < threads; ++t)
        if (pthread_create(&ids[t], NULL, static_worker, &tasks[t]) != 0) die("search_static: pthread_create failed\n");
    SearchResult result = {.score = -INF, .nodes = 1}; // the root itself counts as a node
    for (int t = 0; t < threads; ++t) {
        if (pthread_join(ids[t], NULL) != 0) die("search_static: pthread_join failed\n");
        result.nodes += tasks[t].result.nodes;
        if (tasks[t].result.score > result.score) { // a thread dealt no moves keeps -INF and never wins
            result.score = tasks[t].result.score;
            result.best_move = tasks[t].result.best_move;
            result.has_best_move = true;
        }
    }
    return result;
}
