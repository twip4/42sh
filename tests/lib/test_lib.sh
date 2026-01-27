#!/bin/bash
set -u

TIMEOUT_CMD="timeout 5"
TESTS=0
FAILS=0

run_test() {
    local desc="$1"
    local cmd="$2"
    local expected="${3:-__ANY__}"

    TESTS=$((TESTS + 1))
    echo "  [$TESTS] $desc"

    output=$($TIMEOUT_CMD bash -c "$cmd" 2>&1 || true)

    if [ "$expected" = "__ANY__" ]; then
        echo "ok"
        return
    fi

    if [ "$output" = "$expected" ]; then
        echo "ok"
    else
        echo "fail"
        echo "Expected: '$expected'"
        echo "Got     : '$output'"
        FAILS=$((FAILS + 1))
    fi
}

