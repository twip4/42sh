#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Pipe tests"

run_test "simple pipe" \
    "$BIN_PATH -c 'echo toto | tr a-z A-Z'" \
    "TOTO"

run_test "multiple pipes" \
    "$BIN_PATH -c 'echo a b c | tr a-z A-Z | wc -w'" \
    "3"
