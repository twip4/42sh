#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "export tests"

run_test "export key=value" \
    "$BIN_PATH -c 'export TEST=42; echo \$TEST'" \
    "42"

run_test "export -p prints exports" \
    "$BIN_PATH -c 'export TEST=42; export -p'" \
    "export TEST=42"

run_test "export -p prints from export history" \
    "$BIN_PATH -c 'export TEST_EXPORT=hello; export -p'" \
    "export TEST_EXPORT=hello"

run_test "export invalid format" \
    "$BIN_PATH -c 'export TEST; echo \$?'" \
    "1"

#run_test "unset multiple vars" \
#    "$BIN_PATH -c 'export A=1 B=2; unset A B; echo \"\$A\$B\"'" \
#    ""

