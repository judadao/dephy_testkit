#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=$("$ROOT_DIR/scripts/run_with_result.sh" smoke true)

printf '%s\n' "$OUT"
printf '%s\n' "$OUT" | grep -q '"name":"smoke"'
printf '%s\n' "$OUT" | grep -q '"result":"pass"'
printf '%s\n' "$OUT" | grep -q '"status":0'
printf '%s\n' "$OUT" | grep -q '"duration_ms":'

printf 'result json test passed\n'
