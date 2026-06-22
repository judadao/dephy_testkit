#!/usr/bin/env sh

testkit_fail()
{
    printf 'not ok - %s\n' "$*" >&2
    return 1
}

testkit_ok()
{
    printf 'ok - %s\n' "$*"
}

assert_eq()
{
    expected=$1
    actual=$2
    label=${3:-"expected equality"}

    if [ "$expected" = "$actual" ]; then
        testkit_ok "$label"
        return 0
    fi

    testkit_fail "$label: expected '$expected', got '$actual'"
}

assert_contains()
{
    haystack=$1
    needle=$2
    label=${3:-"expected substring"}

    case "$haystack" in
        *"$needle"*)
            testkit_ok "$label"
            return 0
            ;;
        *)
            testkit_fail "$label: missing '$needle'"
            ;;
    esac
}

assert_file_contains()
{
    file=$1
    needle=$2
    label=${3:-"expected file substring"}

    if [ ! -f "$file" ]; then
        testkit_fail "$label: file not found: $file"
        return 1
    fi

    if grep -Fq "$needle" "$file"; then
        testkit_ok "$label"
        return 0
    fi

    testkit_fail "$label: missing '$needle' in $file"
}

assert_eventually()
{
    command=$1
    timeout=${2:-5}
    interval=${3:-1}
    label=${4:-"eventually true"}
    start=$(date +%s)

    while :; do
        if sh -c "$command" >/dev/null 2>&1; then
            testkit_ok "$label"
            return 0
        fi

        now=$(date +%s)
        if [ $((now - start)) -ge "$timeout" ]; then
            testkit_fail "$label: command did not succeed within ${timeout}s: $command"
            return 1
        fi

        sleep "$interval"
    done
}
