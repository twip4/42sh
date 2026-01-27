#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "If tests"

run_test "if simple" \
    "$BIN_PATH -c 'if echo hi; then echo ok; fi'" \
    $'hi\nok'

run_test "if else" \
    "$BIN_PATH -c 'if false; then echo nope; else echo yep; fi'" \
    "yep"

run_test "if elif else" \
    "$BIN_PATH -c 'if false; then echo no; elif true; then echo yes; else echo no; fi'" \
    "yes"
