#include "search.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// wall clock time in seconds
static double now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

// search a position to a fixed depth and print the best move, score, nodes and time
int main(int argc, char **argv)
{
    bool sequential = argc > 1 && strcmp(argv[1], "sequential") == 0;
    bool parallel = argc > 1 && strcmp(argv[1], "static") == 0;
    int depth = argc > 2 ? atoi(argv[2]) : 0;
    int threads = argc > 3 ? atoi(argv[3]) : 0;
    bool threads_ok = sequential ? threads == 1 : threads >= 1 && threads <= MAX_THREADS;
    if (!(sequential || parallel) || depth < 1 || !threads_ok || argc > 5) {
        fprintf(stderr, "usage is ppchess sequential|static depth threads [fen]\n"
                        "sequential takes 1 thread, static takes 1 to %d, the start position is used when fen is left out\n",
                MAX_THREADS);
        return 1;
    }
    Board board;
    if (argc == 4) board_init(&board);
    else if (!board_from_fen(&board, argv[4])) {
        fprintf(stderr, "bad FEN %s\n", argv[4]);
        return 1;
    }
    board_print(&board);

    double start = now();
    SearchResult result = sequential ? search_sequential(&board, depth) : search_static(&board, depth, threads);
    double seconds = now() - start;

    Move m = result.best_move;
    if (result.has_best_move) printf("best %c%d%c%d", 'a' + m.from % 8, m.from / 8 + 1, 'a' + m.to % 8, m.to / 8 + 1);
    else printf("no legal moves");
    printf("  score %d  nodes %" PRIu64 "  time %.3fs\n", result.score, result.nodes, seconds);
    return 0;
}
