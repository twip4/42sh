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
run_test "cd and pwd" "$BIN_PATH -c 'cd /tmp && pwd'" "/tmp"
