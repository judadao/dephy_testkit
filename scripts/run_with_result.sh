#!/usr/bin/env sh
set -u

if [ "$#" -lt 2 ]; then
    echo "usage: $0 NAME COMMAND [ARG ...]" >&2
    exit 2
fi

name=$1
shift
start=$(date +%s)

"$@"
status=$?
end=$(date +%s)
duration_ms=$(( (end - start) * 1000 ))

escaped_name=$(printf '%s' "$name" | sed 's/\\/\\\\/g; s/"/\\"/g')
if [ "$status" -eq 0 ]; then
    result=pass
else
    result=fail
fi

printf '{"name":"%s","result":"%s","status":%d,"duration_ms":%d}\n' \
    "$escaped_name" "$result" "$status" "$duration_ms"

exit "$status"
