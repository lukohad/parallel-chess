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
- scope change from the proposal. all parallel experiments (prelim and final) run
  in the Hyper-V Ubuntu VM cs550 with 16 virtual CPUs, so T is {1, 2, 4, 8, 12, 16}
  instead of up to 20. 16 leaves the host 12 logical CPUs so Windows doesn't steal
  time from the runs, and the 16 are reserved at 100% (Set-VMProcessor -Reserve 100)
  so Hyper-V guarantees them to the VM instead of sharing them on demand. the same VM setup is used for every experiment, and the
  sequential baseline gets rerun inside it since speedup has to divide times from
  the same environment (the table below is from WSL). the hypervisor decides which
  physical cores the virtual CPUs land on, so above ~8 threads they spill onto
  efficiency cores and hyperthread siblings, the slowdown the proposal predicted.
- sequential baseline, depth 8, in the VM. this is the one speedups divide by.

  | position | nodes | mean | min | max |
  |---|---|---|---|---|
  | start | 8,008,243 | 2.617 s | 2.610 s | 2.630 s |
  | kiwipete | 13,956,106 | 14.775 s | 14.717 s | 14.905 s |
  | position4 | 13,556,938 | 3.505 s | 3.493 s | 3.523 s |
  | position5 | 9,407,837 | 7.541 s | 7.505 s | 7.561 s |

- the same baseline under WSL, within about 1% of the VM, so WSL's numbers were
  fine for development.

  | position | nodes | mean | min | max |
  |---|---|---|---|---|
  | start | 8,008,243 | 2.605 s | 2.585 s | 2.630 s |
  | kiwipete | 13,956,106 | 14.726 s | 14.493 s | 14.896 s |
  | position4 | 13,556,938 | 3.462 s | 3.426 s | 3.489 s |
  | position5 | 9,407,837 | 7.601 s | 7.541 s | 7.675 s |

- static root split race review. shared data is the root board and root move
  list, both read only after the threads start. each thread writes only its own
  StaticTask slot (its node count, score and best move). the main thread reads those
  slots only after pthread_join, which guarantees it sees the thread's writes. no
  shared writes means no locks and no races. also ran it under gcc's
  ThreadSanitizer (-fsanitize=thread) at 2, 8 and 16 threads with zero reports.
  tsan only catches races that actually happen in a run, so the paper argument is
  the real check and tsan backs it up.
- static split correctness. the score matches sequential on every benchmark
  position at every thread count tried, and at 1 thread it searches exactly the
  same nodes. it stays deterministic (same nodes every run) since which thread gets
  which move never changes.
- static split search overhead, early numbers at depth 6 in WSL. kiwipete nodes go
  331,111 (1 thread) -> 416,904 (2) -> 693,804 (4) -> 1,122,947 (8), 3.4x the work
  at 8 threads, because each thread prunes with only its own alpha. this is the
  cost the proposal predicted for static splitting.
- kiwipete runs at ~1M nodes/s versus ~3M for the others. it has far more legal
  moves per node, and the legality check plays and tests every one of them.
- there's no quiescence search, so scores swing at the search horizon (the start
  position reads +100 at depth 5, -100 at 6). this affects move quality, not timing
  or correctness, and the proposal never promised strong play.
- static root split sweep, depth 8, 5 runs per point, VM cs550 (16 vCPUs reserved),
  `for t in 1 2 4 8 12 16; do sh bench.sh static $t; done`. score matched sequential
  at every thread count (start -100, kiwipete -100, position4 -700, position5 200),
  and T=1 searched exactly the sequential node counts.

  raw results, mean time (min-max) and total nodes

  | T | start | kiwipete | position4 | position5 |
  |---|---|---|---|---|
  | 1 | 2.617 s (2.583-2.676), 8,008,243 | 14.753 s (14.675-14.807), 13,956,106 | 3.448 s (3.431-3.469), 13,556,938 | 7.569 s (7.530-7.640), 9,407,837 |
  | 2 | 1.411 s (1.406-1.421), 8,437,844 | 8.574 s (8.530-8.624), 19,084,469 | 2.151 s (2.142-2.164), 17,173,305 | 4.431 s (4.385-4.476), 10,494,601 |
  | 4 | 0.914 s (0.906-0.919), 10,157,102 | 5.982 s (5.963-6.004), 29,720,185 | 1.871 s (1.854-1.883), 26,504,969 | 3.178 s (3.163-3.206), 17,182,757 |
  | 8 | 0.614 s (0.604-0.631), 11,951,279 | 4.167 s (4.112-4.239), 43,105,933 | 1.398 s (1.393-1.406), 28,535,788 | 2.081 s (2.066-2.115), 20,557,482 |
  | 12 | 0.675 s (0.636-0.731), 14,458,723 | 4.264 s (4.219-4.305), 53,708,240 | 1.414 s (1.410-1.421), 28,535,788 | 2.002 s (1.949-2.048), 24,678,536 |
  | 16 | 0.558 s (0.548-0.570), 17,064,863 | 3.637 s (3.562-3.758), 66,843,467 | 1.429 s (1.416-1.437), 28,535,788 | 1.996 s (1.933-2.034), 32,475,176 |

  how the tables below are calculated, all against the VM sequential baseline
  (t_seq = 2.617, 14.775, 3.505, 7.541 s and N_seq = 8,008,243, 13,956,106,
  13,556,938, 9,407,837 for start, kiwipete, position4, position5)
  - speedup S_T = t_seq / t_T, using mean times. e.g. kiwipete at 16 threads is
    14.775 / 3.637 = 4.06
  - efficiency E_T = S_T / T. e.g. kiwipete at 16 is 4.06 / 16 = 0.25
  - search overhead O_T = N_T / N_seq. e.g. kiwipete at 16 is 66,843,467 / 13,956,106 = 4.79
  - position4 at T=1 shows speedup 1.02 because its static T=1 mean (3.448 s) came
    out slightly under the separate sequential run (3.505 s). same nodes, so that's
    run to run noise of under 2%.

  speedup

  | T | start | kiwipete | position4 | position5 |
  |---|---|---|---|---|
  | 1 | 1.00 | 1.00 | 1.02 | 1.00 |
  | 2 | 1.85 | 1.72 | 1.63 | 1.70 |
  | 4 | 2.86 | 2.47 | 1.87 | 2.37 |
  | 8 | 4.26 | 3.55 | 2.51 | 3.62 |
  | 12 | 3.88 | 3.47 | 2.48 | 3.77 |
  | 16 | 4.69 | 4.06 | 2.45 | 3.78 |

  efficiency

  | T | start | kiwipete | position4 | position5 |
  |---|---|---|---|---|
  | 1 | 1.00 | 1.00 | 1.02 | 1.00 |
  | 2 | 0.93 | 0.86 | 0.81 | 0.85 |
  | 4 | 0.72 | 0.62 | 0.47 | 0.59 |
  | 8 | 0.53 | 0.44 | 0.31 | 0.45 |
  | 12 | 0.32 | 0.29 | 0.21 | 0.31 |
  | 16 | 0.29 | 0.25 | 0.15 | 0.24 |

  search overhead

  | T | start | kiwipete | position4 | position5 |
  |---|---|---|---|---|
  | 1 | 1.00 | 1.00 | 1.00 | 1.00 |
  | 2 | 1.05 | 1.37 | 1.27 | 1.12 |
  | 4 | 1.27 | 2.13 | 1.96 | 1.83 |
  | 8 | 1.49 | 3.09 | 2.10 | 2.19 |
  | 12 | 1.81 | 3.85 | 2.10 | 2.62 |
  | 16 | 2.13 | 4.79 | 2.10 | 3.45 |

- static split findings
  - the proposal hoped for speedup above 2. met on every position from T=8 up, best
    4.69x (start, 16 threads).
  - position4 flattens at ~2.5x from T=8 because it only has 6 legal root moves
    (white starts in check). threads 7-16 get dealt nothing, which is why its node
    count is frozen at 28,535,788 for T >= 8. a clean example of the proposal's point
    that root splitting can't use more threads than there are root moves.
  - search overhead is the main cost. kiwipete does 4.79x the sequential work at 16
    threads because each thread prunes only with its own alpha. the dynamic split's
    shared alpha is meant to fix this.
  - start and kiwipete get slower going from 8 to 12 threads, matching the proposal's
    prediction that threads above ~8 land on efficiency cores and hyperthread
    siblings. 16 recovers some since the extra threads spread the work further.
  - efficiency drops to 15-29% at 16 threads. overhead plus load imbalance, one big
    subtree (usually the first, best ordered move) decides when the search finishes
    while the other threads sit idle.
