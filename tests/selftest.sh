#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
. "$ROOT_DIR/scripts/testlib.sh"

TESTKIT_OUT_DIR="$ROOT_DIR/out"

test_begin "testkit selftest"
assert_eq "a" "a" "assert_eq works"
assert_contains "field bridge io" "bridge" "assert_contains works"
assert_eventually "test -f '$ROOT_DIR/scripts/testlib.sh'" 2 1 "assert_eventually works"
port=$(alloc_port 25000 100)
case "$port" in
    ''|*[!0-9]*) testkit_fail "alloc_port returned a number"; exit 1 ;;
    *) testkit_ok "alloc_port returned $port" ;;
esac
test_end
