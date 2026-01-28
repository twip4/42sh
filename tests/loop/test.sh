#!/bin/bash
set -eu

source "$(dirname "$0")/../lib/test_lib.sh"

echo "For loop parser tests"

run_test "for loop with in list" \
    "$BIN_PATH -c 'for i in a b c; do echo \$i; done'" \
    $'a\nb\nc'

run_test "for loop with newlines (parser only)" \
"$BIN_PATH -c '
for i in 1 2
do
echo \$i
done
'" \
""

run_test "for missing variable name" \
    "$BIN_PATH -c 'for ; do echo fail; done'" \
    $'for: expected variable name\nsyntax error !\nerror'

run_test "for missing do" \
    "$BIN_PATH -c 'for i in a b; echo \$i; done'" \
    $'for: expected do\nsyntax error !\nerror'

run_test "for missing done" \
    "$BIN_PATH -c 'for i in a b; do echo \$i;'" \
    $'for: expected done\nsyntax error !\nerror'
