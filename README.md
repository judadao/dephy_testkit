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

## IO Simulator

`io_sim` is a Linux development fixture for industrial IO behavior. It links
against the sibling `dephy_industrial_io` repo by default:

```sh
make io-sim
out/io_sim scenarios/basic_io.sim
```

Override the dependency path when needed:

```sh
make io-sim DEPHY_IO_ROOT=/path/to/dephy_industrial_io
```

Scenario commands:

```text
set <driver_channel> <raw> <advance_ms>
fault <driver_channel> <on|off> <advance_ms>
stuck <driver_channel> <on|off> <raw> <advance_ms>
noise <driver_channel> <raw_span> <advance_ms>
sleep <advance_ms>
write <name> <value>
read <name>
```

The fault/stuck/noise commands operate at the same raw driver boundary that
product firmware will use when it swaps the simulator driver for GPIO, ADC, or
field-bus drivers.

## Planned Fixtures

- MQTT subscriber capture helpers.
- P2P multi-broker startup with per-node logs.
- MQTT bridge integration for `io_sim`.
- TAP/JUnit output for CI.
