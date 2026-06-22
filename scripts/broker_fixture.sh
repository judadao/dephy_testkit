#!/usr/bin/env sh

if [ -z "${TESTKIT_DIR:-}" ]; then
    TESTKIT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
    . "$TESTKIT_DIR/scripts/testlib.sh"
fi

BROKER_FIXTURE_ROOT=${BROKER_FIXTURE_ROOT:-}

broker_fixture_require()
{
    if [ -z "$BROKER_FIXTURE_ROOT" ]; then
        printf 'error: set BROKER_FIXTURE_ROOT to mqtt_min_broker repo path\n' >&2
        return 1
    fi

    test -x "$BROKER_FIXTURE_ROOT/build_out/mqtt_broker"
    test -x "$BROKER_FIXTURE_ROOT/build_out/mqtt_cli"
}

start_broker()
{
    name=$1
    port=${2:-1883}
    log_dir=${TESTKIT_CASE_DIR:-"$PWD/out"}
    log_file="$log_dir/${name}.log"

    broker_fixture_require
    MQTT_BROKER_PORT="$port" "$BROKER_FIXTURE_ROOT/build_out/mqtt_broker" >"$log_file" 2>&1 &
    pid=$!
    testkit_register_pid "$pid"
    wait_port 127.0.0.1 "$port" 5
    printf '%s\n' "$pid"
}

mqtt_pub()
{
    port=$1
    topic=$2
    message=$3

    broker_fixture_require
    "$BROKER_FIXTURE_ROOT/build_out/mqtt_cli" pub -h 127.0.0.1 -p "$port" -t "$topic" -m "$message"
}

