#!/bin/bash

set -e

PROJECT="plotter"
BOARD="arduino:samd:mkr1000"
PORT="/dev/ttyACM0"

cd "$PROJECT"

echo "Compiling..."
arduino-cli compile --fqbn "$BOARD"

echo "Uploading..."
arduino-cli upload -p "$PORT" --fqbn "$BOARD"

echo "Done."
