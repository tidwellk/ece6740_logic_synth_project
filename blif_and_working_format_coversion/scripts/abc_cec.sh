#!/usr/bin/env bash
# Run ABC combinational equivalence checking (CEC) on two BLIF files.
#
# Usage:
#   ./scripts/abc_cec.sh spec.blif impl.blif
#
# Notes:
#   - Update ABC_BIN below to match the local ABC installation.
#   - ABC expects compatible primary input/output interfaces.

set -euo pipefail

ABC_BIN="/Users/shem/Documents/CAD/abc/abc"

if [[ $# -ne 2 ]]; then
    echo "Usage:"
    echo "  ./scripts/abc_cec.sh spec.blif impl.blif"
    exit 1
fi

FILE1="$1"
FILE2="$2"

if [[ ! -x "$ABC_BIN" ]]; then
    echo "Error: ABC executable not found or not executable at:"
    echo "  $ABC_BIN"
    exit 1
fi

if [[ ! -f "$FILE1" ]]; then
    echo "Error: first input file not found:"
    echo "  $FILE1"
    exit 1
fi

if [[ ! -f "$FILE2" ]]; then
    echo "Error: second input file not found:"
    echo "  $FILE2"
    exit 1
fi

FILE1_ABS="$(cd "$(dirname "$FILE1")" && pwd)/$(basename "$FILE1")"
FILE2_ABS="$(cd "$(dirname "$FILE2")" && pwd)/$(basename "$FILE2")"

echo "Spec interface:"
grep '^\.inputs' "$FILE1" || true
grep '^\.outputs' "$FILE1" || true
echo

echo "Impl interface:"
grep '^\.inputs' "$FILE2" || true
grep '^\.outputs' "$FILE2" || true
echo

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

echo "Running ABC CEC:"
echo "  Spec: $FILE1_ABS"
echo "  Impl: $FILE2_ABS"

ABC_CMD="cec \"$FILE1_ABS\" \"$FILE2_ABS\""

if ! (
    cd "$TMP_DIR"
    "$ABC_BIN" -c "$ABC_CMD"
); then
    echo "Error: ABC CEC failed."
    exit 1
fi
