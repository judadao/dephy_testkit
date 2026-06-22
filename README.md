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
out/io_sim --mqtt --site factory-a --node node-7 scenarios/basic_io.sim
out/io_sim --format jsonl scenarios/basic_io.sim
out/io_sim --format mqtt --site factory-a --node node-7 scenarios/basic_io.sim
```

Override the dependency path when needed:

```sh
make io-sim DEPHY_IO_ROOT=/path/to/dephy_industrial_io
```

Scenario commands:

```text
channel <name> <di|do|ai|ao> <driver_channel> <debounce_ms> <scale_num> <scale_den> [offset]
set <driver_channel> <raw> <advance_ms>
fault <driver_channel> <on|off> <advance_ms>
stuck <driver_channel> <on|off> <raw> <advance_ms>
noise <driver_channel> <raw_span> <advance_ms>
sleep <advance_ms>
write <name> <value>
read <name>
expect <name> <value> <fault>
expect_between <name> <min> <max> <fault>
expect_raw <driver_channel> <raw>
```

The fault/stuck/noise commands operate at the same raw driver boundary that
product firmware will use when it swaps the simulator driver for GPIO, ADC, or
field-bus drivers.

If a scenario has no `channel` lines, `io_sim` loads a small built-in default
map for quick smoke tests. If any `channel` line exists, the scenario-defined
map replaces the defaults.

With `--mqtt`, every IO event also emits deterministic MQTT-like lines:

```text
mqtt site/<site>/node/<node>/io/<channel>/event {...}
mqtt site/<site>/node/<node>/io/<channel>/state {...}
```

Use `--format jsonl` for machine-readable event/read/assertion records, or
`--format mqtt` for MQTT command stream output only.

## Planned Fixtures

- MQTT subscriber capture helpers.
- P2P multi-broker startup with per-node logs.
- MQTT bridge integration for `io_sim`.
- TAP/JUnit output for CI.
