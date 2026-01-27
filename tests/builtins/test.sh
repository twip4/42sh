#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Builtin tests"

run_test "cd and pwd" \
    "$BIN_PATH -c 'cd /tmp && pwd'" \
    "/tmp"

run_test "pwd" \
    "$BIN_PATH -c 'pwd'" \
    "$(pwd)"

echo "true / false"

run_test "true returns success" \
    "$BIN_PATH -c 'true; echo \$?'" \
    "0"

run_test "false returns failure" \
    "$BIN_PATH -c 'false; echo \$?'" \
    "1"

echo "cd tests"

run_test "cd to /tmp" \
    "$BIN_PATH -c 'cd /tmp && pwd'" \
    "/tmp"

run_test "cd invalid path" \
    "$BIN_PATH -c 'cd /does_not_exist; echo \$?'" \
    "cd: /does_not_exist: No such file or directory
1"

run_test "cd HOME" \
    "$BIN_PATH -c 'cd; pwd'" \
    "$HOME"

run_test "cd dash (OLDPWD)" \
    "$BIN_PATH -c 'cd /tmp; cd -; pwd'" \
    "$(pwd)"

