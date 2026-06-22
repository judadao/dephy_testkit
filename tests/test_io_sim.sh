#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=$("$ROOT_DIR/out/io_sim" "$ROOT_DIR/scenarios/basic_io.sim")
FAULT_OUT=$("$ROOT_DIR/out/io_sim" "$ROOT_DIR/scenarios/fault_io.sim")
MAP_OUT=$("$ROOT_DIR/out/io_sim" "$ROOT_DIR/scenarios/channel_map.sim")
MQTT_OUT=$("$ROOT_DIR/out/io_sim" --mqtt --site factory-a --node node-7 "$ROOT_DIR/scenarios/basic_io.sim")
JSONL_OUT=$("$ROOT_DIR/out/io_sim" --format jsonl "$ROOT_DIR/scenarios/basic_io.sim")
MQTT_ONLY_OUT=$("$ROOT_DIR/out/io_sim" --format mqtt --site factory-a --node node-7 "$ROOT_DIR/scenarios/basic_io.sim")
NOISE_OUT=$("$ROOT_DIR/out/io_sim" "$ROOT_DIR/scenarios/noise_io.sim")

printf '%s\n' "$OUT"
printf '%s\n' "$FAULT_OUT"
printf '%s\n' "$MAP_OUT"
printf '%s\n' "$MQTT_OUT"
printf '%s\n' "$JSONL_OUT"
printf '%s\n' "$MQTT_ONLY_OUT"
printf '%s\n' "$NOISE_OUT"

printf '%s\n' "$OUT" | grep -q 'read door di 0 0 0'
printf '%s\n' "$OUT" | grep -q 'event rising door di 1 0 20'
printf '%s\n' "$OUT" | grep -q 'read pressure_ma_x100 ai 1200 0 21'
printf '%s\n' "$OUT" | grep -q 'event changed relay do 1 0 21'
printf '%s\n' "$OUT" | grep -q 'event falling door di 0 0 41'
printf '%s\n' "$FAULT_OUT" | grep -q 'event fault pressure_ma_x100 ai 0 1 0'
printf '%s\n' "$FAULT_OUT" | grep -q 'read pressure_ma_x100 ai 0 1 0'
printf '%s\n' "$FAULT_OUT" | grep -q 'read pressure_ma_x100 ai 700 0 3'
printf '%s\n' "$FAULT_OUT" | grep -q 'read pressure_ma_x100 ai 1200 0 5'
printf '%s\n' "$MAP_OUT" | grep -q 'read limit_switch di 0 0 0'
printf '%s\n' "$MAP_OUT" | grep -q 'event rising limit_switch di 1 0 5'
printf '%s\n' "$MAP_OUT" | grep -q 'read tank_level ai 200 0 6'
printf '%s\n' "$MAP_OUT" | grep -q 'event changed valve_setpoint ao 70 0 6'
printf '%s\n' "$MAP_OUT" | grep -q 'event changed pump_cmd do 1 0 6'
printf '%s\n' "$MQTT_OUT" | grep -q 'mqtt site/factory-a/node/node-7/io/door/event {"event":"rising","type":"di","value":1,"fault":0,"t_ms":20}'
printf '%s\n' "$MQTT_OUT" | grep -q 'mqtt site/factory-a/node/node-7/io/pressure_ma_x100/state {"type":"ai","value":1200,"fault":0,"t_ms":21}'
printf '%s\n' "$JSONL_OUT" | grep -q '{"kind":"event","event":"rising","name":"door","type":"di","value":1,"fault":0,"t_ms":20}'
printf '%s\n' "$JSONL_OUT" | grep -q '{"kind":"ok","assert":"expect","name":"relay","value":1,"fault":0}'
printf '%s\n' "$MQTT_ONLY_OUT" | grep -q 'mqtt site/factory-a/node/node-7/io/door/event {"event":"rising","type":"di","value":1,"fault":0,"t_ms":20}'
if printf '%s\n' "$MQTT_ONLY_OUT" | grep -q '^event '; then
    printf 'unexpected text event output in mqtt format\n' >&2
    exit 1
fi
printf '%s\n' "$NOISE_OUT" | grep -q 'ok expect_between noisy_pressure 800 1200 0'
printf '%s\n' "$NOISE_OUT" | grep -q 'ok expect noisy_pressure 1000 0'

printf 'io_sim scenario test passed\n'
