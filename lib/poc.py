# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Chang Liu

import os
import subprocess
from pathlib import Path

def build(poc):
    makefile_dir = Path(__file__).resolve().parent.parent / "src" / f"pitfall-{poc}"
    makefile = f"{poc}-poc.make"
    cmd = f"make -C {makefile_dir} -f {makefile} clean && make -C {makefile_dir} -f {makefile}"
    subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return

def run(cpu, poc):
    elf = Path(__file__).resolve().parent.parent / "bin" / f"pitfall-{poc}-poc"
    cmd = f"taskset -c {cpu} {elf}"
    os.system(cmd)

def exp(cpu, poc):
    build(poc)
    run(cpu, poc)

if __name__ == "__main__":
    exp(3, 'v2')