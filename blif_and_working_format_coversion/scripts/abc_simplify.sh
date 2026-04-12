#!/usr/bin/env bash
set -euo pipefail

ABC_BIN="/Users/shem/Documents/CAD/abc/abc"

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage:"
    echo "  ./scripts/abc_simplify.sh input.blif"
    echo "  ./scripts/abc_simplify.sh input.blif output.blif"
    exit 1
fi

INPUT_BLIF="$1"
OUTPUT_BLIF="${2:-}"

if [[ ! -x "$ABC_BIN" ]]; then
    echo "Error: ABC executable not found or not executable at:"
    echo "  $ABC_BIN"
    exit 1
fi

if [[ ! -f "$INPUT_BLIF" ]]; then
    echo "Error: input BLIF file not found:"
    echo "  $INPUT_BLIF"
    exit 1
fi

INPUT_ABS="$(cd "$(dirname "$INPUT_BLIF")" && pwd)/$(basename "$INPUT_BLIF")"

if [[ -z "$OUTPUT_BLIF" ]]; then
    BASENAME="$(basename "$INPUT_BLIF")"
    STEM="${BASENAME%.*}"
    OUTPUT_BLIF="./${STEM}_simplified.blif"
fi

OUTPUT_DIR="$(dirname "$OUTPUT_BLIF")"
OUTPUT_BASE="$(basename "$OUTPUT_BLIF")"
mkdir -p "$OUTPUT_DIR"
OUTPUT_ABS="$(cd "$OUTPUT_DIR" && pwd)/$OUTPUT_BASE"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

echo "Running ABC on: $INPUT_ABS"
echo "Writing simplified BLIF to: $OUTPUT_ABS"

ABC_CMD="read_blif \"$INPUT_ABS\"; strash; balance; rewrite; refactor; rewrite; balance; renode; write_blif \"$OUTPUT_ABS\""

if ! (
    cd "$TMP_DIR"
    "$ABC_BIN" -c "$ABC_CMD"
); then
    echo "Error: ABC simplification failed."
    exit 1
fi

if [[ ! -f "$OUTPUT_ABS" ]]; then
    echo "Error: ABC finished without creating output file:"
    echo "  $OUTPUT_ABS"
    exit 1
fi

echo "ABC simplification finished."
