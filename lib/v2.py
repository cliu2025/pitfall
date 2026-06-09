# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Chang Liu

import json
import os
import re
import subprocess
from tqdm import tqdm
from pathlib import Path

def build(test_byte_size):
    makefile_dir = Path(__file__).resolve().parent.parent / "src" / "pitfall-v2"
    makefile = "v2.make"
    cmd = f"make -C {makefile_dir} -f {makefile} clean && make -C {makefile_dir} -f {makefile} test_byte_size={test_byte_size}"
    # print(cmd)
    subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return

def run(cpu):
    elf = Path(__file__).resolve().parent.parent / "bin" / "pitfall-v2-eval"
    cmd = f"taskset -c {cpu} {elf}"
    e = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
    pat = re.compile(
        r'acc\s*=\s*(?P<acc>[0-9]+(?:\.[0-9]+)?),\s*'
        r'spend\s*(?P<spend>[0-9]+(?:\.[0-9]+)?)\s*s\s*\n'
        r'throughput:\s*(?P<throughput>[0-9]+(?:\.[0-9]+)?)\s*Bps\n'
        r'# of victim function call:\s*(?P<num_calls>[0-9]+(?:\.[0-9]+)?)'
    )
    m = pat.search(e.stdout)
    if m:
        data = [float(m.group('acc')), float(m.group('spend')), float(m.group('throughput')), float(m.group('num_calls'))]
    else:
        data = []
    return data

def exp(cpu, byte_size):
    print("Evaluation on Pitfall-v2")
    v2_data = []
    for i in tqdm(range(0, 10), ncols=80, dynamic_ncols=True, leave=False):
        build(byte_size)
        data = run(cpu)
        if (len(data) == 4):
            v2_data.append(data)
    data_file = Path(__file__).resolve().parent.parent / "data" / "v2.json"
    with open(data_file, "w") as f:
        json.dump(v2_data, f)
    best_through_of_all = max(v2_data, key=lambda x: x[2])
    print(f"Evaluation of Pitfall-v2:\nAccuracy: {best_through_of_all[0]}\nThroughput: {best_through_of_all[2]}\nNumber of Function calls: {best_through_of_all[3]}")
    print("------------------\n")

if __name__ == "__main__":
    exp(3, 1000)