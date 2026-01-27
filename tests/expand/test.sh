#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Expansion tests"

run_test "expand \$\$ (pid)" \
    "$BIN_PATH -c \"echo \$\$\""

run_test "expand \$# (argc)" \
    "$BIN_PATH -c \"echo \$#\""

run_test "expand RANDOM" \
    "$BIN_PATH -c \"echo \$RANDOM\""

run_test "expand UID" \
    "$BIN_PATH -c \"echo \$UID\"" \
    "$(id -u)"

run_test "expand \$0" \
    "$BIN_PATH -c \"echo \$0\""

run_test "expand \$1 out of range" \
    "$BIN_PATH -c \"echo \$1\"" \
    ""

run_test "expand \$*" \
    "$BIN_PATH -c \"echo \$*\""

run_test "expand \$@" \
    "$BIN_PATH -c \"echo \$@\""

