#!/usr/bin/env bash
set -euo pipefail

ALEXA_N="${1:-10}"
SITES_LIST="alexa${ALEXA_N}"
echo "Using sites list: $SITES_LIST"
export DISPLAY="${DISPLAY:-:1}"


CHROME_VERSION="149.0.7827.55"
CFT_BASE_URL="https://storage.googleapis.com/chrome-for-testing-public/${CHROME_VERSION}/linux64"

echo "========================================"
echo "[0/3] Start setup"
echo "Working directory: $PWD"
echo "Chrome version: $CHROME_VERSION"
echo "========================================"

echo ""
echo "========================================"
echo "[1/3] Install system packages"
echo "========================================"
sudo apt update
sudo apt install -y wget unzip ca-certificates python3 python3-pip python3-venv python3-full build-essential

echo ""
echo "========================================"
echo "[2/3] Download Chrome and ChromeDriver"
echo "========================================"
mkdir -p "$PWD/fingerprinting/chrome_path"

echo "Downloading Chrome..."
wget -O "$PWD/fingerprinting/chrome_path/chrome-linux64.zip" "$CFT_BASE_URL/chrome-linux64.zip"

echo "Downloading ChromeDriver..."
wget -O "$PWD/fingerprinting/chrome_path/chromedriver-linux64.zip" "$CFT_BASE_URL/chromedriver-linux64.zip"

echo ""
echo "========================================"
echo "[3/3] Extract Chrome and ChromeDriver"
echo "========================================"
rm -rf "$PWD/fingerprinting/chrome_path/chrome-linux64"
rm -rf "$PWD/fingerprinting/chrome_path/chromedriver-linux64"

unzip -o "$PWD/fingerprinting/chrome_path/chrome-linux64.zip" -d "$PWD/fingerprinting/chrome_path"
unzip -o "$PWD/fingerprinting/chrome_path/chromedriver-linux64.zip" -d "$PWD/fingerprinting/chrome_path"

echo "Creating executable shortcuts..."
ln -sf "$PWD/fingerprinting/chrome_path/chrome-linux64/chrome" "$PWD/fingerprinting/chrome_path/chrome"
ln -sf "$PWD/fingerprinting/chrome_path/chromedriver-linux64/chromedriver" "$PWD/fingerprinting/chrome_path/chromedriver"

chmod +x "$PWD/fingerprinting/chrome_path/chrome-linux64/chrome"
chmod +x "$PWD/fingerprinting/chrome_path/chromedriver-linux64/chromedriver"

echo "Chrome version:"
"$PWD/fingerprinting/chrome_path/chrome" --version

echo "ChromeDriver version:"
"$PWD/fingerprinting/chrome_path/chromedriver" --version
