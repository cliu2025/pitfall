# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Chang Liu

import json
import os
import re
import subprocess
from tqdm import tqdm
from pathlib import Path

def exp(number_of_sites):
    print("Evaluation on Pitfall-v3")

    cur_dir = os.getcwd()
    script_dir = Path(__file__).resolve().parent.parent / "fingerprinting" / "scripts"
    os.chdir(script_dir)

    os.system(f"./run.sh {number_of_sites}")

    print("------------------\n")

    os.chdir(cur_dir)

if __name__ == "__main__":
    exp(4)