#!/usr/bin/env bash
# One-command build + flash for the CYD D&D firmware (macOS/Linux).
# Installs PlatformIO if it's missing, then builds and uploads over USB.
# Run from anywhere: bash tools/flash.sh
set -e
cd "$(dirname "$0")/.."

if ! command -v pio >/dev/null 2>&1; then
    echo "PlatformIO not found - installing (pip install -U platformio)..."
    pip3 install -U platformio || pip install -U platformio
fi

echo "Building and flashing... (connect the board over USB first)"
pio run -t upload

echo
echo "Done. Don't forget to copy sd_card_data/ onto the microSD card - see README.md."
