#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=$("$ROOT_DIR/out/io_sim" "$ROOT_DIR/scenarios/basic_io.sim")
FAULT_OUT=$("$ROOT_DIR/out/io_sim" "$ROOT_DIR/scenarios/fault_io.sim")

printf '%s\n' "$OUT"
printf '%s\n' "$FAULT_OUT"

printf '%s\n' "$OUT" | grep -q 'read door di 0 0 0'
printf '%s\n' "$OUT" | grep -q 'event rising door di 1 0 20'
printf '%s\n' "$OUT" | grep -q 'read pressure_ma_x100 ai 1200 0 21'
printf '%s\n' "$OUT" | grep -q 'event changed relay do 1 0 21'
printf '%s\n' "$OUT" | grep -q 'event falling door di 0 0 41'
printf '%s\n' "$FAULT_OUT" | grep -q 'event fault pressure_ma_x100 ai 0 1 0'
printf '%s\n' "$FAULT_OUT" | grep -q 'read pressure_ma_x100 ai 0 1 0'
printf '%s\n' "$FAULT_OUT" | grep -q 'read pressure_ma_x100 ai 700 0 3'
printf '%s\n' "$FAULT_OUT" | grep -q 'read pressure_ma_x100 ai 1200 0 5'

printf 'io_sim scenario test passed\n'
