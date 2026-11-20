# C Chess Engine Skeleton

A minimal C11 foundation for the CS 450 Parallel Programming project.
Requires GCC and Make; no external libraries are needed.

From this directory:

```sh
make
./ppchess sequential 5 1
./ppchess static 8 4 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
make test
make bench
make clean
```

`ppchess sequential|static depth threads [fen]` prints the position (the start
position if no FEN is given), searches it to `depth` plies with the chosen version
and prints the best move, score, node count and wall time. sequential takes 1
thread, static takes 1 to 64. Uppercase
letters represent White, lowercase letters represent Black, and dots are empty
squares. The board uses a 64-square array with `a1 = 0` and `h8 = 63`.
Castling rights use the four bit masks in `include/board.h`; `en_passant = -1`
means no target square is available.

- `include/board.h`, `src/board.c` hold the board representation, initialization, printing and FEN loading (`board_from_fen`).

  `board_from_fen` loads any position from a FEN string. the experiments need it
  since the proposal benchmarks a fixed set of FEN positions. validation is light on
  purpose. every FEN we load is a constant in our own code, so it only checks that
  the piece layout fills exactly 64 squares.

- `include/movegen.h`, `src/movegen.c` hold legal move generation and `make_move`.

  all of move generation lives in this one file.

  `make_move` copies the board into `next` and only writes to the copy. the
  original is `const`, so the compiler won't let it change. this matters most for
  the parallel versions. every thread can read the same root position with no lock
  since nobody writes to it, and each thread builds its own child boards on its own
  stack. with make/unmake (change one board in place, undo it after) each thread
  would need its own private board, and an undo bug turns into a race or a wrong
  score. it also keeps the legality check and recursion simple, a child board is
  just dropped when we're done with it and there's nothing to undo. the cost is
  copying ~280 bytes per move, which is the same for every version so it doesn't
  skew the speedup numbers.

  moves are generated per piece ignoring check, then each one is played and kept
  only if our own king isn't attacked afterward. that handles pins, checks and en
  passant discovered checks with no special cases. it's slower than tracking pins
  directly but much harder to get wrong. castling is the exception, the king can't
  castle out of or through check, and the filter only sees where the king lands, so
  those squares are checked up front.

  black pieces are white pieces + 6 in the `Piece` enum, so `type_of` maps a black
  piece to its white version and the `WHITE_*` values double as piece types. that
  saves a separate piece type enum.

  a castling right is cleared once its king or rook is off its home square. that
  covers the king moving, the rook moving and the rook being captured with no
  separate cases.

- `include/search.h`, `src/search.c` hold the search result type, material evaluation (`evaluate`, pawn 100, knight and bishop 300, rook 500, queen 900 centipawns) the sequential search (`search_sequential`) and the static root split (`search_static`).

  the search is negamax with alpha-beta, the same recursion as
  `document/baseline/alphabeta.c` but on real positions. a position with no legal
  moves scores as checkmate or stalemate (via `in_check` in movegen), and a sooner
  mate scores higher so the search goes for the fastest one. the node count goes
  through the `SearchResult` passed down the recursion instead of a global, so each
  thread in the parallel versions can count its own nodes without a race. timing
  lives in `main.c` around the search call (`gettimeofday`, same as in class), so the
  search itself has no timer in it.

  moves are searched captures first. sorting captures by victim value (MVV-LVA)
  prunes better, but captures first is simpler to read.

  the static root split orders the root moves, deals them round-robin to T threads
  (thread t gets moves t, t + T, t + 2T...) and each thread searches its moves with
  its own private alpha. after the join the best score wins and node counts are
  summed. the only shared data is the root board and the root move list, and both
  are only read, so there are no locks. each thread writes just its own task slot,
  and the main thread reads those only after `pthread_join`, which guarantees it sees
  everything the thread wrote. at 1 thread it searches exactly the same nodes as the
  sequential version. with more threads each one prunes less since it can't see the
  others' alpha, so total nodes go up (search overhead) while time goes down. on a
  tie the lowest numbered thread wins, so the printed best move can differ from
  sequential but the score never does.

- `src/main.c` reads the command line, runs the search and times it.

The dynamic root split and the hybrid remain unfinished.

Everything builds with `-O2 -pthread`.

## testing

`make test` builds and runs `tests/perft.c` (about 1 s) and exits nonzero if any
count is wrong.

perft counts every legal move sequence from a position down to a fixed depth. the
totals for a few standard positions are published, so if our counts match, the
move generator is right. it runs the start position, Kiwipete and positions 3-5
from the [chessprogramming perft results](https://www.chessprogramming.org/Perft_Results),
which together cover castling, en passant, promotions and pins. the engine never
uses it. it's here because every benchmark counts nodes, and those numbers only
mean something if the move tree is correct.

the test positions are loaded with `board_from_fen` because the start position
never reaches castling, promotions or tricky en passant at low depth. a mistyped
FEN gives a wrong count too, so perft also catches bad positions, which is why
`board_from_fen` can get away with light validation.

## benchmarks

`make bench` runs `bench.sh`, which searches the start position, Kiwipete and
perft positions 4 and 5 at depth 8, 5 times each, and prints the score, node count
and mean, min and max time per position. `sh bench.sh version threads depth runs`
overrides the defaults (sequential, 1 thread, depth 8, 5 runs), so a thread sweep
is `for t in 1 2 4 8 12 16; do sh bench.sh static $t; done`. the score should match
the sequential one for every version and thread count. perft position 3 is left out since it finishes in about 0.1 s at depth
8, too fast to time reliably. node counts are the same every run because the
sequential and static searches are deterministic, so only time gets min and max. `.gitattributes` keeps
`bench.sh` on LF line endings so it still runs from a Windows checkout.

`results/` holds saved `bench.sh` output from the VM, one file per version:
`sequential.txt` is `sh bench.sh sequential` and `static.txt` is the static thread
sweep, `for t in 1 2 4 8 12 16; do sh bench.sh static $t; done > results/static.txt`.
save new versions the same way. the report figures and tables are generated from
these files by `document/prelim_proposal/figures/plot_results.py`. the current two
files were transcribed from the tables in `document/report-notes.md`, since the
original terminal output wasn't saved.

the positions all come from the [chessprogramming perft results](https://www.chessprogramming.org/Perft_Results)
page, the standard set for checking move generators.

- start is the normal opening position, 20 legal moves.
- kiwipete is a crowded middlegame made to stress move generators, with castling
  on both sides, pins, en passant and promotions all possible. 48 legal moves.
- position4 is a sharp middlegame where white starts in check and both sides have
  a pawn one step from promoting.
- position5 is a middlegame with a white pawn on d7 about to promote, known for
  catching bugs in other engines.

sequential baseline, depth 8, 5 runs, i7-14700K, Hyper-V Ubuntu VM cs550 with 16
virtual CPUs reserved at 100%. every speedup is measured against these times. the
same run under WSL came out within about 1%.

| position | nodes | mean | min | max |
|---|---|---|---|---|
| start | 8,008,243 | 2.617 s | 2.610 s | 2.630 s |
| kiwipete | 13,956,106 | 14.775 s | 14.717 s | 14.905 s |
| position4 | 13,556,938 | 3.505 s | 3.493 s | 3.523 s |
| position5 | 9,407,837 | 7.541 s | 7.505 s | 7.561 s |
