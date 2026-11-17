# C Chess Engine Skeleton

A minimal C11 foundation for the CS 450 Parallel Programming project.
Requires GCC and Make; no external libraries are needed.

From this directory:

```sh
make
./chess-engine
make test
make clean
```

The program initializes and prints the standard starting position. Uppercase
letters represent White, lowercase letters represent Black, and dots are empty
squares. The board uses a 64-square array with `a1 = 0` and `h8 = 63`.
Castling rights use the four bit masks in `include/board.h`; `en_passant = -1`
means no target square is available.

- `include/board.h`, `src/board.c`: board representation, initialization, printing.
- `include/movegen.h`, `src/movegen.c`: move types and unimplemented move functions.
- `include/search.h`, `src/search.c`: search result type and sequential search stub.
- `src/main.c`: starting-position demonstration.
- `tests/perft.c`: perft placeholder.

Move generation, move application, evaluation, search, perft, and multithreading
remain unfinished. Calling an unimplemented function prints an error and aborts
instead of returning a fabricated result. Move flags are reserved for future use.

`make test` compiles the perft placeholder, reports that tests are unimplemented,
and exits with a nonzero status. It does not run or claim passing chess tests.
