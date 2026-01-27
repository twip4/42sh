#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Logic Tests"

run_test "OR: left fails, right runs" \
    "$BIN_PATH -c \"false || echo right\"" \
    "right"

run_test "OR: left succeeds, right skipped" \
    "$BIN_PATH -c \"true || echo right\"" \
    ""
run_test "AND: left succeeds, right runs" \
    "$BIN_PATH -c \"true && echo right\"" \
    "right"

run_test "AND: left fails, right skipped" \
    "$BIN_PATH -c \"false && echo right\"" \
    ""

run_test "subshell output" \
    "$BIN_PATH -c \"(echo hello)\"" \
    "hello"

run_test "AND + OR combo 1" \
    "$BIN_PATH -c \"false && echo a || echo b\"" \
    "b"

run_test "AND + OR combo 2" \
    "$BIN_PATH -c \"true && echo a || echo b\"" \
    "a"

run_test "complex sequence with subshell" \
    "$BIN_PATH -c \"echo start; (echo sub1 && echo sub2) || echo fallback; echo end\"" \
    $'start\nsub1\nsub2\nend'
