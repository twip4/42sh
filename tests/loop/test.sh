#!/bin/bash
set -eu

source "$(dirname "$0")/../lib/test_lib.sh"

echo "while / until tests"

run_test "while simple loop" \
    "$BIN_PATH -c 'i=0; while [ \$i -lt 3 ]; do echo \$i; i=\$((i+1)); done'" \
    $'while: expected done\nsyntax error !\nerror'

run_test "until simple loop" \
    "$BIN_PATH -c 'i=0; until [ \$i -eq 3 ]; do echo \$i; i=\$((i+1)); done'" \
    $'until: expected done\nsyntax error !\nerror'

run_test "while invalid condition" \
    "$BIN_PATH -c 'while ; do echo test; done'" \
    $'syntax error !\nerror'

run_test "while missing do" \
    "$BIN_PATH -c 'while true echo test; done'" \
    $'while: expected do\nsyntax error !\nerror'

run_test "while missing done" \
    "$BIN_PATH -c 'while true; do echo test'" \
    $'while: expected done\nsyntax error !\nerror'

run_test "until invalid body" \
    "$BIN_PATH -c 'until true; do ; done'" \
    $'until: invalid body\nsyntax error !\nerror'

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

run_test "for without in uses g_shell.argv" \
    "$BIN_PATH arg1 arg2 -c 'for VAR; do echo \$VAR; done'" \
    $'arg1\narg2'
