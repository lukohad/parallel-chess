"""Plots speedup, efficiency and search overhead from saved bench.sh output.

usage: python plot_results.py BASELINE VERSION [VERSION ...]

BASELINE is the output of `sh bench.sh sequential`. Each VERSION file is the output of a
thread sweep such as `for t in 1 2 4 8 12 16; do sh bench.sh static $t; done`. For each
version this writes <version>_speedup, <version>_efficiency and <version>_overhead as PNG
and PDF, plus <version>_times.tex (mean time table with the baseline as the first row),
all next to this script.
"""
import re
import sys
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

# TrueType embedding. The default Type 3 fonts render with broken kerning in PDF viewers.
plt.rcParams["pdf.fonttype"] = 42

OUT_DIR = Path(__file__).parent
LABELS = {"start": "Start", "kiwipete": "Kiwipete", "position4": "Position 4", "position5": "Position 5"}

# The exact lines bench.sh prints: one header per run, then one row per position.
HEADER = re.compile(r"^(\w+), (\d+) threads, depth (\d+), (\d+) runs$")
ROW = re.compile(r"^(\S+)\s+score (-?\d+)\s+nodes (\d+)\s+mean ([\d.]+)s\s+min ([\d.]+)s\s+max ([\d.]+)s$")


def parse(path):
    """Returns (version, depth, {threads: {position: (score, nodes, mean)}})."""
    version = depth = threads = None
    runs = {}
    for number, line in enumerate(Path(path).read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip():
            continue
        if header := HEADER.match(line):
            if version not in (None, header[1]) or depth not in (None, int(header[3])):
                sys.exit(f"{path}:{number}: mixes versions or depths in one file")
            version, threads, depth = header[1], int(header[2]), int(header[3])
            runs[threads] = {}
        elif (row := ROW.match(line)) and threads is not None:
            runs[threads][row[1]] = (int(row[2]), int(row[3]), float(row[4]))
        else:
            sys.exit(f"{path}:{number}: not bench.sh output: {line!r}")
    if not runs:
        sys.exit(f"{path}: no bench.sh runs found")
    for t, rows in runs.items():
        if set(rows) != set(LABELS):
            sys.exit(f"{path}: {t} threads has positions {sorted(rows)}, expected {sorted(LABELS)}")
    return version, depth, runs


def plot(name, title, ylabel, threads, series, ytick_step, ideal=None):
    fig, ax = plt.subplots(figsize=(3.5, 1.85), layout="constrained")
    ax.set_title(title, fontsize=10)
    if ideal is not None:
        ax.plot(threads, [ideal] * len(threads), "k--", linewidth=0.8, label="Ideal")
    for pos, label in LABELS.items():
        ax.plot(threads, series[pos], marker="o", markersize=3, label=label)
    ax.set_xlabel("Threads")
    ax.set_ylabel(ylabel)
    ax.set_xticks(threads)
    ax.yaxis.set_major_locator(MultipleLocator(ytick_step))
    ax.grid(alpha=0.3)
    # Legend sits below the figure so it never covers a line, all entries on one row.
    fig.legend(fontsize=7, loc="outside lower center", ncol=len(LABELS) + (ideal is not None),
               frameon=False, handlelength=1.2, columnspacing=0.8)
    for ext in ("png", "pdf"):
        fig.savefig(OUT_DIR / f"{name}.{ext}", dpi=300, bbox_inches="tight", pad_inches=0.02)
    plt.close(fig)


def write_times_table(name, baseline, runs):
    rows = [("Seq", baseline)] + [(str(t), runs[t]) for t in sorted(runs)]
    lines = [r"\begin{tabular}{|c|c|c|c|c|}", r"\hline",
             "T & " + " & ".join(LABELS.values()).replace("Position", "Pos.") + r" \\", r"\hline"]
    for label, results in rows:
        lines.append(label + " & " + " & ".join(f"{results[p][2]:.3f}" for p in LABELS) + r" \\")
        if label == "Seq":
            lines.append(r"\hline")
    lines += [r"\hline", r"\end{tabular}"]
    (OUT_DIR / f"{name}_times.tex").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    base_version, base_depth, base_runs = parse(sys.argv[1])
    if base_version != "sequential" or list(base_runs) != [1]:
        sys.exit(f"{sys.argv[1]}: baseline must be a single `bench.sh sequential` run")
    baseline = base_runs[1]
    for path in sys.argv[2:]:
        version, depth, runs = parse(path)
        if depth != base_depth:
            sys.exit(f"{path}: depth {depth} does not match baseline depth {base_depth}")
        for t, rows in runs.items():
            for pos, (score, _, _) in rows.items():
                if score != baseline[pos][0]:
                    sys.exit(f"{path}: {pos} at {t} threads scored {score}, sequential scored {baseline[pos][0]}")
        threads = sorted(runs)
        speedup = {p: [baseline[p][2] / runs[t][p][2] for t in threads] for p in LABELS}
        efficiency = {p: [s / t for s, t in zip(speedup[p], threads)] for p in LABELS}
        overhead = {p: [runs[t][p][1] / baseline[p][1] for t in threads] for p in LABELS}
        title = version.replace("_", " ").title() + (" Root Split" if version in ("static", "dynamic") else "")
        plot(f"{version}_speedup", f"{title} Speedup", "Speedup", threads, speedup, ytick_step=1)
        plot(f"{version}_efficiency", f"{title} Efficiency", "Efficiency", threads, efficiency,
             ytick_step=0.2, ideal=1)
        plot(f"{version}_overhead", f"{title} Search Overhead", "Search overhead", threads, overhead, ytick_step=1)
        write_times_table(version, baseline, runs)


main()
