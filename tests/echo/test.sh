#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Echo tests"

run_test "simple echo" \
    "$BIN_PATH -c 'echo hello'" \
    "hello"

run_test "echo multiple args" \
    "$BIN_PATH -c 'echo a b c'" \
    "a b c"

run_test "echo -n" \
    "$BIN_PATH -c 'echo -n hello; echo world'" \
    "helloworld"

run_test "echo -e newline" \
    "$BIN_PATH -c 'echo -e \"a\\nb\"'" \
    $'a\nb'

run_test "echo -e tab" \
    "$BIN_PATH -c 'echo -e \"a\\tb\"'" \
    $'a\tb'

run_test "echo -e backslash" \
    "$BIN_PATH -c 'echo -e \"\\\\\"'" \
    "\\"

run_test "echo -E disables escapes" \
    "$BIN_PATH -c 'echo -E \"a\\nb\"'" \
    "a\\nb"

run_test "echo unknown option treated as arg" \
    "$BIN_PATH -c 'echo -x test'" \
    "-x test"
