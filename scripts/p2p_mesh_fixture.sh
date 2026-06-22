#!/usr/bin/env sh

if [ -z "${TESTKIT_DIR:-}" ]; then
    TESTKIT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
    . "$TESTKIT_DIR/scripts/testlib.sh"
fi

mesh_static_seed_list()
{
    ports=$*
    first=1
    out=

    for port in $ports; do
        if [ "$first" -eq 1 ]; then
            out="127.0.0.1:$port"
            first=0
        else
            out="$out,127.0.0.1:$port"
        fi
    done

    printf '%s\n' "$out"
}

mesh_note()
{
    printf '[mesh] %s\n' "$*"
}

p2p_mesh_start()
{
    count=$1
    base_mqtt=${2:-21000}
    base_p2p=${3:-22000}
    out_dir=${TESTKIT_CASE_DIR:-"$PWD/out"}
    ports=
    p2p_ports=
    i=0

    if ! command -v broker_build >/dev/null 2>&1; then
        . "$TESTKIT_DIR/scripts/broker_fixture.sh"
    fi

    while [ "$i" -lt "$count" ]; do
        ports="$ports $((base_mqtt + i))"
        p2p_ports="$p2p_ports $((base_p2p + i))"
        i=$((i + 1))
    done

    i=0
    for mqtt_port in $ports; do
        p2p_port=$((base_p2p + i))
        broker_bin=$(broker_build "mesh_$i" "$mqtt_port" "$p2p_port" 1 "$out_dir/mesh_$i")
        seeds=
        for peer_p2p in $p2p_ports; do
            if [ "$peer_p2p" != "$p2p_port" ]; then
                if [ -z "$seeds" ]; then
                    seeds="127.0.0.1:$peer_p2p"
                else
                    seeds="$seeds,127.0.0.1:$peer_p2p"
                fi
            fi
        done
        MQTT_P2P_PEERS="$seeds" "$broker_bin" >"$out_dir/mesh_$i.log" 2>&1 &
        pid=$!
        testkit_register_pid "$pid"
        wait_port 127.0.0.1 "$mqtt_port" 5
        printf '%s %s %s\n' "$i" "$mqtt_port" "$p2p_port"
        i=$((i + 1))
    done
}
