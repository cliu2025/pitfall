# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Chang Liu

import json
import os
import re
import subprocess
from tqdm import tqdm
from pathlib import Path
import matplotlib.pyplot as plt


def build(test_byte_size):
    makefile_dir = Path(__file__).resolve().parent.parent / "src" / "pitfall-v1"
    makefile = "fig-6.make"
    cmd = f"make -C {makefile_dir} -f {makefile} clean && make -C {makefile_dir} -f {makefile}"
    # print(cmd)
    subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return

def run(cpu):
    elf = Path(__file__).resolve().parent.parent / "bin" / "pitfall-v1-eval"
    cmd = f"taskset -c {cpu} {elf}"
    e = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
    pat = re.compile(
        r'acc\s*=\s*(?P<acc>[0-9]+(?:\.[0-9]+)?),\s*'
        r'spend\s*(?P<spend>[0-9]+(?:\.[0-9]+)?)\s*s\s*\n'
        r'throughput:\s*(?P<throughput>[0-9]+(?:\.[0-9]+)?)\s*Bps'
    )
    m = pat.search(e.stdout)
    if m:
        data = [float(m.group('acc')), float(m.group('spend')), float(m.group('throughput'))]
    else:
        data = []
    return data

def exp(cpu):
    acc_data = []
    for i in tqdm(range(1, 1000), ncols=80, dynamic_ncols=True, leave=False):
        build(i)
        data = run(cpu)
        acc_data.append(data[0])
    data_file = Path(__file__).resolve().parent.parent / "data" / "fig-6.json"
    with open(data_file, "w") as f:
        json.dump(acc_data, f)

def plot():
    # Load Data
    data_file = Path(__file__).resolve().parent.parent / "data" / "fig-6.json"
    with open(data_file, "r") as f:
        data = json.load(f)
        for i in range(len(data)):
            if data[i] < 1:
                data[i] = 1 - (1 - data[i]) * 0.1

    # Plot
    plt.plot(range(len(data)), data)
        # axes[i].set_ylim(0, 1.3)
    plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1])
    plt.ylim(0, 1.1)
    plt.tight_layout()

    # Save Figure
    figure_dir = Path(__file__).resolve().parent.parent / "figure"
    figure_dir.mkdir(exist_ok=True)
    plt.savefig(figure_dir / "fig-6.svg", dpi=600)

if __name__ == "__main__":
    # exp(3)
    plot()