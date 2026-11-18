#include "movegen.h"

#include <inttypes.h>
#include <stdio.h>

// count leaf nodes of the full legal move tree to check movegen against known totals
static uint64_t perft(const Board *board, int depth)
{
    if (depth == 0) return 1;
    MoveList list;
    generate_legal_moves(board, &list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; ++i) {
        Board next;
        make_move(board, list.moves[i], &next);
        nodes += perft(&next, depth - 1);
    }
    return nodes;
}

// reference counts from https://www.chessprogramming.org/Perft_Results
static const struct { const char *fen; int depth; uint64_t nodes; } CASES[] = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609},
    {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603},
    {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5, 674624},
    {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333},
    {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487},
};

// run every perft case, exit nonzero if any count is wrong
int main(void)
{
    int failures = 0;
    for (size_t i = 0; i < sizeof CASES / sizeof CASES[0]; ++i) {
        Board board;
        if (!board_from_fen(&board, CASES[i].fen)) {
            printf("FAIL  bad FEN: %s\n", CASES[i].fen);
            ++failures;
            continue;
        }
        uint64_t nodes = perft(&board, CASES[i].depth);
        bool ok = nodes == CASES[i].nodes;
        failures += !ok;
        printf("%s  depth %d  %" PRIu64 " (expected %" PRIu64 ")  %s\n",
               ok ? "PASS" : "FAIL", CASES[i].depth, nodes, CASES[i].nodes, CASES[i].fen);
    }
    return failures == 0 ? 0 : 1;
}
