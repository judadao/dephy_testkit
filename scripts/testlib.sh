#!/usr/bin/env sh

set -eu

TESTKIT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
. "$TESTKIT_DIR/scripts/assert.sh"

TESTKIT_OUT_DIR=${TESTKIT_OUT_DIR:-"$PWD/out"}
TESTKIT_CASE_DIR=
TESTKIT_PIDS=

test_begin()
{
    TESTKIT_CASE_NAME=$1
    safe_name=$(printf '%s' "$TESTKIT_CASE_NAME" | tr -c 'A-Za-z0-9_.-' '_')
    TESTKIT_CASE_DIR="$TESTKIT_OUT_DIR/$safe_name"
    mkdir -p "$TESTKIT_CASE_DIR"
    printf '=== %s ===\n' "$TESTKIT_CASE_NAME"
}

test_end()
{
    printf '=== done: %s ===\n' "${TESTKIT_CASE_NAME:-test}"
}

testkit_register_pid()
{
    TESTKIT_PIDS="${TESTKIT_PIDS:-} $1"
}

testkit_cleanup()
{
    for pid in ${TESTKIT_PIDS:-}; do
        if kill -0 "$pid" >/dev/null 2>&1; then
            kill "$pid" >/dev/null 2>&1 || true
        fi
    done
}

trap testkit_cleanup EXIT INT TERM

wait_port()
{
    host=$1
    port=$2
    timeout=${3:-5}
    start=$(date +%s)

    while :; do
        if command -v nc >/dev/null 2>&1 && nc -z "$host" "$port" >/dev/null 2>&1; then
            return 0
        fi

        now=$(date +%s)
        if [ $((now - start)) -ge "$timeout" ]; then
            return 1
        fi

        sleep 0.1
    done
}

alloc_port()
{
    base=${1:-20000}
    span=${2:-20000}
    i=0

    while [ "$i" -lt "$span" ]; do
        port=$((base + i))
        if ! command -v nc >/dev/null 2>&1 || ! nc -z 127.0.0.1 "$port" >/dev/null 2>&1; then
            printf '%s\n' "$port"
            return 0
        fi
        i=$((i + 1))
    done

    return 1
}
