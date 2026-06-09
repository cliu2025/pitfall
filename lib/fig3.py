# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Chang Liu

import json
import subprocess
from pathlib import Path
from tqdm import tqdm
import numpy as np
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt


def build(st_aligned_bits, ld_aligned_bits):
    makefile_dir = Path(__file__).resolve().parent.parent / "src" / "org"
    makefile = "fig-3.make"
    cmd = f"make -C {makefile_dir} -f {makefile} clean && make -C {makefile_dir} -f {makefile} align_size_st={(1 << st_aligned_bits)} align_size_ld={(1 << ld_aligned_bits)}"
    subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return

def run(cpu):
    microbenchmark = Path(__file__).resolve().parent.parent / "bin" / "microbenchmark-fig-3"
    cmd = f"taskset -c {cpu} {microbenchmark}"
    e = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
    data = int(e.stdout)
    return data

def exp(cpu):
    data_file = Path(__file__).resolve().parent.parent / "data" / "fig-3.json"
    collided_data = []
    for i in tqdm(range(1, 20), ncols=80, dynamic_ncols=True, leave=False):
        for j in range(1, 20):
            build(i, j)
            data = run(cpu)
            collided_data.append([i, j, data])
    with open(data_file, "w") as f:
        json.dump(collided_data, f)

def plot():
    # Format Configuration
    plt.rcParams.update({
        'xtick.labelsize': 14,
        'ytick.labelsize': 14,
    })
    colors = ["#FFF7DE", "#005F8C"]
    cmap_color = matplotlib.colors.LinearSegmentedColormap.from_list("green_blue", colors)

    # Load Data
    data_file = Path(__file__).resolve().parent.parent / "data" / "fig-3.json"
    with open(data_file, 'r') as f:
        data = json.load(f)
    max_i = max(x[0] for x in data)
    max_j = max(x[1] for x in data)
    mat = np.full((max_i + 1, max_j + 1), 0)
    for i, j, k in data:
        mat[i, j] = k
    vmin = np.nanmin(mat)
    vmax = np.nanmax(mat)
    mat_norm = (mat - vmin) / (vmax - vmin)

    # Plot
    fig = plt.figure(figsize=(10, 8))
    ax_ = sns.heatmap(
        mat_norm,
        vmin=0,
        vmax=1,
        cmap=cmap_color
    )
    ax_.invert_yaxis()
    plt.xlabel("Aligned LSBs of Store PC", fontsize=16)
    plt.ylabel("Aligned LSBs of Load PC", fontsize=16)
    colorbar = ax_.collections[0].colorbar
    colorbar.ax.set_xlabel("Entry Sharing Rate", fontsize=16, labelpad=24)

    # Save Figure
    figure_dir = Path(__file__).resolve().parent.parent / "figure"
    figure_dir.mkdir(exist_ok=True)
    plt.savefig(figure_dir / "fig-3.svg", dpi=600)

if __name__ == "__main__":
    exp(3)
    plot()