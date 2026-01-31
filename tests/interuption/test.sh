#!/bin/bash
set -eu

source "$(dirname "$0")/../lib/test_lib.sh"

echo "break / continue builtin tests"

run_test "break outside loop" \
    "$BIN_PATH -c 'break; echo \$?'" \
    $'not in a loop\n1'

run_test "break inside for loop" \
    "$BIN_PATH -c 'for i in a b c; do echo \$i; break; echo fail; done'" \
    "a"

run_test "break stops loop immediately" \
    "$BIN_PATH -c 'for i in 1 2 3; do echo \$i; break; done'" \
    "1"

run_test "continue outside loop" \
    "$BIN_PATH -c 'continue; echo \$?'" \
    $'not in a loop\n1'

run_test "continue inside for loop" \
    "$BIN_PATH -c 'for i in 1 2 3; do echo \$i; continue; echo fail; done'" \
    $'1\n2\n3'

run_test "continue skips rest of body" \
    "$BIN_PATH -c 'for i in 1 2 3; do echo start-\$i; continue; echo end-\$i; done'" \
    $'start-1\nstart-2\nstart-3'

run_test "break after continue is not reached" \
    "$BIN_PATH -c 'for i in 1 2 3; do continue; break; done; echo done'" \
    "done"

run_test "break only breaks inner loop" \
    "$BIN_PATH -c 'for i in 1 2; do for j in a b; do echo \$i\$j; break; done; done'" \
    $'1a\n2a'

run_test "continue only continues inner loop" \
    "$BIN_PATH -c 'for i in 1 2; do for j in a b; do continue; echo fail; done; echo outer-\$i; done'" \
    $'outer-1\nouter-2'
