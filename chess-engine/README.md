# C Chess Engine Skeleton

A minimal C11 foundation for the CS 450 Parallel Programming project.
Requires GCC and Make; no external libraries are needed.

From this directory:

```sh
make
./ppchess
make test
make clean
```

The program initializes and prints the standard starting position. Uppercase
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

- `include/search.h`, `src/search.c`: search result type and sequential search stub.
- `src/main.c`: starting-position demonstration.

Evaluation, search and multithreading remain unfinished. Calling an unimplemented
function prints an error and aborts instead of returning a fabricated result.

Everything builds with `-O2`.

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
