#!/bin/bash
set -eu
source "$(dirname "$0")/../lib/test_lib.sh"

echo "Builtin tests"

run_test "cd and pwd" \
    "$BIN_PATH -c 'cd /tmp && pwd'" \
    "/tmp"

run_test "pwd" \
    "$BIN_PATH -c 'pwd'" \
    "$(pwd)"

echo "true / false"

run_test "true returns success" \
    "$BIN_PATH -c 'true; echo \$?'" \
    "0"

run_test "false returns failure" \
    "$BIN_PATH -c 'false; echo \$?'" \
    "1"

echo "cd tests"

run_test "cd to /tmp" \
    "$BIN_PATH -c 'cd /tmp && pwd'" \
    "/tmp"

run_test "cd invalid path" \
    "$BIN_PATH -c 'cd /does_not_exist; echo \$?'" \
    $'cd: /does_not_exist: No such file or directory\n1'

run_test "cd HOME" \
    "$BIN_PATH -c 'cd; pwd'" \
    "$HOME"

run_test "cd dash (OLDPWD)" \
    "$BIN_PATH -c 'cd /tmp; cd -; pwd'" \
    "$(pwd)"

# Dot tests

echo "Dot tests"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/simple.sh" << 'EOF'
echo hello
echo world
EOF

run_test "dot simple script" \
    "$BIN_PATH -c '. $TMP_DIR/simple.sh'" \
    $'hello\nworld'

cat > "$TMP_DIR/env.sh" << 'EOF'
export DOT_VAR=hello
EOF

run_test "dot preserves environment variable" \
    "$BIN_PATH -c '. $TMP_DIR/env.sh; echo \$DOT_VAR'" \
    "hello"

run_test "dot without argument" \
    "$BIN_PATH -c '.'" \
    ".: filename argument required"

echo "Exit / is_number tests"

run_test "exit with numeric argument" \
    "$BIN_PATH -c 'exit 42'"

run_test "exit with non-numeric argument" \
    "$BIN_PATH -c 'exit abc'"

echo "Unset builtin tests"

run_test "unset single variable" \
    "$BIN_PATH -c 'export MYVAR=hello; unset MYVAR; echo \$MYVAR'" \
    ""

run_test "unset multiple variables" \
    "$BIN_PATH -c 'export VAR1=one; export VAR2=two; unset VAR1 VAR2; echo \$VAR1 \$VAR2'" \
    " "
