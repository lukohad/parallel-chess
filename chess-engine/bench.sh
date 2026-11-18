#!/bin/sh
# search every benchmark position RUNS times at DEPTH and print nodes and mean, min and max time
# usage is ./bench.sh [depth] [runs], defaults are depth 8 and 5 runs
DEPTH=${1:-8}
RUNS=${2:-5}

# name and FEN, the perft positions that take seconds at depth 8
while read -r name fen; do
    for run in $(seq "$RUNS"); do ./ppchess "$DEPTH" "$fen" | tail -n 1; done |
    # last line of ppchess is "best m score s nodes n time t", t ends in "s" which +0 drops
    awk -v name="$name" '
        { t = $NF + 0; nodes = $(NF - 2); sum += t }
        NR == 1 || t < min { min = t }
        t > max { max = t }
        END { printf "%-10s nodes %-10s mean %.3fs  min %.3fs  max %.3fs\n", name, nodes, sum / NR, min, max }'
done <<EOF
start rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
kiwipete r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
position4 r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1
position5 rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8
EOF
