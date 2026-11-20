# Parallel Chess

This project is intended to implement a *chess engine* utilizing several different parallel optimizations on its evaluation algorithm and evaluate their effectiveness.

## Background

A chess engine is a program that analyzes a game state and chooses a move.

One common algorithm for move selection is *Alpha-Beta* pruning, an optimization of the *Minimax* algorithm for selecting the best move in a competetive environment.

This algorithm has some inherent challenges in its parallelization due to a partially sequential limitation enforced on it due to *shared alpha-beta states* across the search tree.

Despite this limitation, there are many different types of algorithms built to parallelize *alpha-beta* pruning.

## Recurring Challenges

Different algorithms will face different challenges. There are several overarching challenges for parallelizing *alpha-beta pruning*, inherent to a tree-search algorithm:

 - **Idle processors:** naively splitting work between several processors may result in a great deal of *idle time* while processors wait for dependencies to be resolved
 - **Workload imbalance:** skewed trees may result in imbalanced workloads between tasks and create a poor parallel optimization
 - **Synchronization overhead:** since parallel executions are very dependent on the tree's *shared alpha-beta states*, there must be a sophisticated synchronization technique as to not incur excessive overhead
 - **Shared structure contention:** data structures used to synchronize state can create bottlenecks when multiple workers attempt to interact with them
 - **Redundancy:** some optimizations result in several workers performing some of the same job, resuling in greater parallelism but also greater redundancy

## Parallel Algorithms

### Dynamic Tree Splitting

### ABDADA


## Proposal

The project proposal is in `document/proposal/`, built from the course IEEE template. Build with `make` (or `pdflatex paper && bibtex paper && pdflatex paper && pdflatex paper`) inside that folder; output is `document/proposal/paper.pdf`. `document/proposal/paper_original.tex` is the first draft, kept for reference.

`document/report-notes.md` collects notes for the prelim and final reports.

## Preliminary report

The preliminary report is in `document/prelim_proposal/` (IEEE template, same as the proposal). Build with `make` (pdflatex, bibtex, pdflatex twice) inside that folder; output is `document/prelim_proposal/paper.pdf`. References are in `document/prelim_proposal/biblio.bib`. Figures and the mean time table come from `figures/plot_results.py` (matplotlib), which reads saved `bench.sh` output from `chess-engine/results/` (see the engine README). Run it as `python figures/plot_results.py BASELINE VERSION...`, or `make figures` for the current sequential and static files. For each version it writes `<version>_speedup`, `<version>_efficiency` and `<version>_overhead` as PNG and PDF, plus `<version>_times.tex`, which `paper.tex` pulls in with `\input`. It exits with an error if a file isn't `bench.sh` output, if depths differ, or if any score differs from the sequential score.

## Sequential Baseline

`document/baseline/alphabeta.c` is a single-threaded negamax alpha-beta search over a synthetic game tree: every node has the same number of children, and leaf scores are a hash of the node id, so a given (depth, branching, seed) always produces the same tree. It is the preliminary benchmark for the proposal.

```
gcc -O2 -o alphabeta document/baseline/alphabeta.c
./alphabeta 6 35 1    # depth 6, 35 children per node, seed 1
```
