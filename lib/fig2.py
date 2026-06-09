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

def build():
    makefile_dir = Path(__file__).resolve().parent.parent / "src" / "fig-2"
    makefile = "fig-2.make"
    cmd = f"make -C {makefile_dir} -f {makefile} clean && make -C {makefile_dir} -f {makefile}"
    subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return

def run(cpu):
    microbenchmark = Path(__file__).resolve().parent.parent / "bin" / "microbenchmark-fig-2"
    cmd = f"taskset -c {cpu} {microbenchmark}"
    e = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
    data = e.stdout
    return data

def exp(cpu):
    build()
    data_init = run(cpu)
    data = [int(i) for i in data_init.strip().split(" ")]
    data_file = Path(__file__).resolve().parent.parent / "data" / "fig-2.json"
    with open(data_file, "w") as f:
        json.dump(data, f)

def plot():
    # Load Data
    data_file = Path(__file__).resolve().parent.parent / "data" / "fig-2.json"
    with open(data_file, "r") as f:
        data = json.load(f)

    # Plot
    plt.plot(range(len(data)), data)
    plt.xlabel("Timing Sample ID", fontsize=16)
    plt.ylabel("CPU Cycles", fontsize=16)

    # Save figure
    figure_dir = Path(__file__).resolve().parent.parent / "figure"
    figure_dir.mkdir(exist_ok=True)
    plt.savefig(figure_dir / "fig-2.svg", dpi=600)

if __name__ == "__main__":
    exp(3)
    plot()