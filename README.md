# dephy_testkit

Shared shell test harness for Dephy modules and product repositories.

The first version focuses on process-based Linux integration tests: assertions,
port allocation, log directories, cleanup traps, broker helpers, and P2P mesh
fixture hooks. Product and module repositories can source these scripts instead
of duplicating setup and cleanup logic in every test.

## Use

```sh
. /path/to/dephy_testkit/scripts/testlib.sh

test_begin "example"
assert_eq "ready" "ready" "state is ready"
test_end
```

## Self Test

```sh
make test
```

## Planned Fixtures

- MQTT subscriber capture helpers.
- P2P multi-broker startup with per-node logs.
- Industrial IO simulator helper around `dephy_industrial_io`.
- TAP/JUnit output for CI.

