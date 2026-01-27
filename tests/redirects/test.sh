#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Redirect tests"

run_test "output >" \
    "$BIN_PATH -c 'echo hello > out.txt; cat out.txt'" \
    "hello"

run_test "append >>" \
    "$BIN_PATH -c 'echo first >> out.txt; echo second >> out.txt; cat out.txt'" \
    $'hello\nfirst\nsecond' #il y a le hello avec le output davant

echo "input test" > in.txt

run_test "input <" \
    "$BIN_PATH -c 'cat < in.txt'" \
    "input test"

run_test "read/write <>" \
    "$BIN_PATH -c 'echo rw test <> rw.txt; cat rw.txt'" \
    "rw test"

echo "Redirect pipe (>|) tests"

run_test ">| pipe simple" \
    "$BIN_PATH -c 'cat'" \
    "hello" \
    <<< "hello"

run_test ">| pipe multiple lines" \
    "$BIN_PATH -c 'cat'" \
    $'line1\nline2' \
    <<< $'line1\nline2'

run_test ">| pipe empty" \
    "$BIN_PATH -c 'cat'" \
    "" \
    <<< ""
