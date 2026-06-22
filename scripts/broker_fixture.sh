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

broker_build()
{
    name=$1
    mqtt_port=${2:-1883}
    p2p_port=${3:-4884}
    p2p=${4:-0}
    out_dir=${5:-"${TESTKIT_OUT_DIR:-$PWD/out}/broker_$name"}

    if [ -z "$BROKER_FIXTURE_ROOT" ]; then
        printf 'error: set BROKER_FIXTURE_ROOT to mqtt_min_broker repo path\n' >&2
        return 1
    fi

    mkdir -p "$out_dir"
    if [ "$p2p" = "1" ]; then
        make -C "$BROKER_FIXTURE_ROOT" -f Makefile.linux \
            OUTDIR="$out_dir" P2P=1 MQTT_PORT="$mqtt_port" P2P_PORT="$p2p_port" all >/dev/null
    else
        make -C "$BROKER_FIXTURE_ROOT" -f Makefile.linux \
            OUTDIR="$out_dir" MQTT_PORT="$mqtt_port" all >/dev/null
    fi

    printf '%s/mqtt_broker\n' "$out_dir"
}

start_broker()
{
    name=$1
    port=${2:-1883}
    broker_bin=${3:-}
    log_dir=${TESTKIT_CASE_DIR:-"$PWD/out"}
    log_file="$log_dir/${name}.log"

    if [ -z "$broker_bin" ]; then
        broker_fixture_require
        broker_bin="$BROKER_FIXTURE_ROOT/build_out/mqtt_broker"
    fi

    "$broker_bin" >"$log_file" 2>&1 &
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

mqtt_capture_start()
{
    port=$1
    topic=$2
    outfile=$3
    seconds=${4:-5}
    client_id=${5:-"testkit_capture_$$"}

    broker_fixture_require
    timeout "$seconds" "$BROKER_FIXTURE_ROOT/build_out/mqtt_cli" \
        sub -h 127.0.0.1 -p "$port" -i "$client_id" -t "$topic" >"$outfile" 2>"$outfile.err" &
    pid=$!
    testkit_register_pid "$pid"
    printf '%s\n' "$pid"
}

mqtt_capture_wait_for()
{
    outfile=$1
    needle=$2
    timeout_s=${3:-5}

    assert_eventually "grep -F '$needle' '$outfile'" "$timeout_s" 1 \
        "mqtt capture contains $needle"
}
