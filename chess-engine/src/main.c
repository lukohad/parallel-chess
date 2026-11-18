#include "search.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
    int depth = argc > 1 ? atoi(argv[1]) : 0;
    if (depth < 1 || argc > 3) {
        fputs("usage is ppchess depth [fen], the start position is used when fen is left out\n", stderr);
        return 1;
    }
    Board board;
    if (argc == 2) board_init(&board);
    else if (!board_from_fen(&board, argv[2])) {
        fprintf(stderr, "bad FEN %s\n", argv[2]);
        return 1;
    }
    board_print(&board);

    double start = now();
    SearchResult result = search_sequential(&board, depth);
    double seconds = now() - start;

    Move m = result.best_move;
    if (result.has_best_move) printf("best %c%d%c%d", 'a' + m.from % 8, m.from / 8 + 1, 'a' + m.to % 8, m.to / 8 + 1);
    else printf("no legal moves");
    printf("  score %d  nodes %" PRIu64 "  time %.3fs\n", result.score, result.nodes, seconds);
    return 0;
}
