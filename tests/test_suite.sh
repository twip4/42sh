#!/bin/bash
set -eu

BIN_PATH="$1"
OUTPUT_FILE="$2"
COVERAGE="$3"

TIMEOUT_CMD="timeout 5"
TESTS=0

run_test() {
    local desc="$1"
    local cmd="$2"
    local expected="${3:-}"

    TESTS=$((TESTS + 1))
    echo "Tests N$TESTS"
    output=$($TIMEOUT_CMD bash -c "$cmd" 2>&1 || true)

    if [ -n "$expected" ]; then
        if [ "$output" = "$expected" ]; then
            echo "ok"
        else
            echo "fail"
            echo "  Expected: '$expected'"
            echo "  Got     : '$output'"
        fi
    else
        if [ $? -eq 0 ]; then
            echo "ok"
        else
            echo "fail"
            echo "  Command exited with error."
            echo "  Output: '$output'"
        fi
    fi
}

if [ "$COVERAGE" = "yes" ]; then
    echo "unit:"
    run_test "echo hello (unit)" "$BIN_PATH -c 'echo hello'" "hello"
    run_test "pwd (unit)" "$BIN_PATH -c 'pwd'" "$(pwd)"
fi

# Functional
echo "functional:"
run_test "echo simple" "$BIN_PATH -c 'echo toto'" "toto"
run_test "echo with quotes" "$BIN_PATH -c 'echo \"Toto | tata\"'" "Toto | tata"

# Redirects tests

run_test "redirect output >" "$BIN_PATH -c 'echo hello > out.txt; cat out.txt'" "hello"
echo "" > out.txt

run_test "redirect append >>" "$BIN_PATH -c 'echo first >> out.txt; echo second >> out.txt; cat out.txt'" $'first\nsecond'

echo "input test" > in.txt

run_test "redirect input <" "$BIN_PATH -c 'cat < in.txt'" "input test"

run_test "pipe" "$BIN_PATH -c 'echo toto | tr a-z A-Z'" "TOTO"

run_test "dup output >&" "$BIN_PATH -c 'echo err >&2'" ""

echo "dup input test" > in.txt

run_test "dup input <&" "$BIN_PATH -c 'cat <&0 < in.txt'" "dup input test"

run_test "read/write <>" "$BIN_PATH -c 'echo rw test <> rw.txt; cat rw.txt'" "rw test"
