stuff to note in the prelim and final reports, collected while building the engine.

- the engine is deliberately simple (material eval, no transposition table, no
  null move or late move pruning), so its absolute speed is far below production
  engines like Stockfish, which reach depth 20-30+ in the time we take for depth 8.
  we measure speedup against our own sequential baseline, which isolates the effect
  of parallelization. don't compare absolute numbers to other engines.
- move generation is verified with perft against published node counts on five
  standard positions (start, Kiwipete, positions 3-5), covering castling, en
  passant, promotions and pins. every benchmark counts nodes, so this is what makes
  those numbers trustworthy.
- make_move is copy-make. the parent board is never written, so threads can share
  the root position with no lock and each builds its own child boards on its own
  stack. costs ~280 bytes copied per move, same for every version so speedup isn't
  skewed. node counts travel in SearchResult instead of a global for the same reason.
- alpha-beta checked against the same search with pruning disabled, depth 4 on
  three positions. same best move and score every time, 50-370x fewer nodes.
- captures-first move ordering, depth 6, same best move and score with fewer nodes.
  start 664,922 -> 297,470, Kiwipete 13,663,041 -> 331,111, position 5
  10,585,904 -> 237,546 (up to 45x fewer). MVV-LVA would prune more but captures
  first was picked for readability. ordering was added before collecting any
  baseline numbers so the baseline didn't have to be redone.
- the sequential search is deterministic, same nodes and score every run. that's
  what lets parallel results be checked against it, and why only time gets
  min/max in the baseline.
- benchmark setup is depth 8, 5 runs, i7-14700K (20 physical cores, 28 logical)
  under WSL Ubuntu, gcc 15.2 -O2, timed with gettimeofday. run to run spread is
  about 2%. perft position 3 was dropped from the benchmark set since it finishes
  in ~0.1 s at depth 8, too fast to time reliably.
- sequential baseline, depth 8

  | position | nodes | mean | min | max |
  |---|---|---|---|---|
  | start | 8,008,243 | 2.605 s | 2.585 s | 2.630 s |
  | kiwipete | 13,956,106 | 14.726 s | 14.493 s | 14.896 s |
  | position4 | 13,556,938 | 3.462 s | 3.426 s | 3.489 s |
  | position5 | 9,407,837 | 7.601 s | 7.541 s | 7.675 s |

- kiwipete runs at ~1M nodes/s versus ~3M for the others. it has far more legal
  moves per node, and the legality check plays and tests every one of them.
- there's no quiescence search, so scores swing at the search horizon (the start
  position reads +100 at depth 5, -100 at 6). this affects move quality, not timing
  or correctness, and the proposal never promised strong play.
