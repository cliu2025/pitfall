#!/usr/bin/env bash
set -euo pipefail

CHROME_VERSION="149.0.7827.55"
CFT_BASE_URL="https://storage.googleapis.com/chrome-for-testing-public/${CHROME_VERSION}/linux64"

echo "========================================"
echo "[0/7] Start setup"
echo "Working directory: $PWD"
echo "Chrome version: $CHROME_VERSION"
echo "========================================"

echo ""
echo "========================================"
echo "[1/7] Install system packages"
echo "========================================"
sudo apt update
sudo apt install -y wget unzip ca-certificates python3 python3-pip python3-venv python3-full build-essential

echo ""
echo "========================================"
echo "[2/7] Download Chrome and ChromeDriver"
echo "========================================"
mkdir -p "$PWD/chrome_path"

echo "Downloading Chrome..."
wget -c -O "$PWD/chrome_path/chrome-linux64.zip" "$CFT_BASE_URL/chrome-linux64.zip"

echo "Downloading ChromeDriver..."
wget -c -O "$PWD/chrome_path/chromedriver-linux64.zip" "$CFT_BASE_URL/chromedriver-linux64.zip"

echo ""
echo "========================================"
echo "[3/7] Extract Chrome and ChromeDriver"
echo "========================================"
rm -rf "$PWD/chrome_path/chrome-linux64"
rm -rf "$PWD/chrome_path/chromedriver-linux64"

unzip -o "$PWD/chrome_path/chrome-linux64.zip" -d "$PWD/chrome_path"
unzip -o "$PWD/chrome_path/chromedriver-linux64.zip" -d "$PWD/chrome_path"

echo "Creating executable shortcuts..."
ln -sf "$PWD/chrome_path/chrome-linux64/chrome" "$PWD/chrome_path/chrome"
ln -sf "$PWD/chrome_path/chromedriver-linux64/chromedriver" "$PWD/chrome_path/chromedriver"

chmod +x "$PWD/chrome_path/chrome-linux64/chrome"
chmod +x "$PWD/chrome_path/chromedriver-linux64/chromedriver"

echo "Chrome version:"
"$PWD/chrome_path/chrome" --version

echo "ChromeDriver version:"
"$PWD/chrome_path/chromedriver" --version

echo ""
echo "========================================"
echo "[4/7] Prepare Python environment"
echo "========================================"
python3 -m venv .pitfall
source .pitfall/bin/activate

echo "Python path:"
which python

echo "Python version:"
python --version

echo "Upgrading pip, setuptools, and wheel..."
python -m pip install --upgrade pip setuptools wheel

echo "Installing Python requirements..."
python -m pip install -r requirements.txt

echo ""
echo "========================================"
echo "[5/7] Build Pitfall code"
echo "========================================"
make clean
make test

echo ""
echo "========================================"
echo "[6/7] Collect data"
echo "========================================"
export DISPLAY="${DISPLAY:-:1}"
echo "DISPLAY: $DISPLAY"


echo "Starting record.py..."
python3 record.py --sites_list alexa10 --num_runs 100 --out_directory default --browser chrome --chrome_binary_path ./chrome_path --disable_chrome_sandbox True --twilio_interval 0

echo ""
echo "========================================"
echo "[7/7] Analyze data"
echo "========================================"
python3 scripts/check_results.py --data_file default

echo ""
echo "========================================"
echo "Done"
echo "========================================"
