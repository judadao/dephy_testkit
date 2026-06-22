#!/usr/bin/env sh

if [ -z "${TESTKIT_DIR:-}" ]; then
    TESTKIT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
    . "$TESTKIT_DIR/scripts/testlib.sh"
fi

mesh_static_seed_list()
{
    shift_count=$1
    shift
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

