#!/usr/bin/env bash
set -euo pipefail
cd ..

ALEXA_N="${1:-10}"
SITES_LIST="alexa${ALEXA_N}"
echo "Using sites list: $SITES_LIST"
export DISPLAY="${DISPLAY:-:1}"


echo "Chrome version:"
"$PWD/chrome_path/chrome" --version

echo "ChromeDriver version:"
"$PWD/chrome_path/chromedriver" --version

echo ""
echo "========================================"
echo "[0/3] Prepare Python environment"
echo "========================================"

# source "$HOME/miniconda3/etc/profile.d/conda.sh"
# conda activate pitfall-env

# Should use conda
echo "Python path:"
which python

echo "Python version:"
python --version

# echo "Upgrading pip, setuptools, and wheel..."
# python -m pip install --upgrade pip setuptools wheel

# echo "Installing Python requirements..."
# python -m pip install -r requirements.txt

echo ""
echo "========================================"
echo "[1/3] Build Pitfall code"
echo "========================================"
make clean
make test

echo ""
echo "========================================"
echo "[2/3] Collect data"
echo "========================================"
export DISPLAY="${DISPLAY:-:1}"
echo "DISPLAY: $DISPLAY"


echo "Starting record.py..."
python3 record.py --sites_list ${SITES_LIST} --num_runs 100 --out_directory default --browser chrome --chrome_binary_path ./chrome_path --disable_chrome_sandbox True --twilio_interval 0

echo ""
echo "========================================"
echo "[3/3] Analyze data"
echo "========================================"
python3 scripts/check_results.py --data_file default
mv default/fig8.png ../../figure/fig8.png
echo ""
echo "========================================"
echo "Done. Please check figure/fig8.png"
echo "========================================"
