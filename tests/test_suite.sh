#!/bin/bash
set -eu

BIN_PATH="$1"
COVERAGE="${2:-no}"

export BIN_PATH COVERAGE

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "================================="
echo " Running all test suites"
echo "================================="

TOTAL_FAILS=0

for suite in "$ROOT_DIR"/*/test.sh; do
    echo
    echo "Suite: $(basename "$(dirname "$suite")")"
    echo "---------------------------------"
    bash "$suite" || TOTAL_FAILS=$((TOTAL_FAILS + 1))
done

echo
echo "================================="
if [ "$TOTAL_FAILS" -eq 0 ]; then
    echo "All test suites passed"
else
    echo "$TOTAL_FAILS suite(s) failed"
fi
