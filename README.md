# dephy_testkit

Reusable Linux test harness and IO simulator fixtures for Dephy repos.

Use this repo for shell assertions, process cleanup, broker/P2P fixtures,
industrial IO scenarios, and machine-readable test result wrappers.

## Layout

```text
scripts/assert.sh              assertion helpers
scripts/testlib.sh             test lifecycle, cleanup, ports, eventually
scripts/run_with_result.sh     JSON result wrapper
scripts/broker_fixture.sh      broker helper functions
scripts/p2p_mesh_fixture.sh    static-seed P2P mesh helper
tools/io_sim.c                 industrial IO scenario runner
scenarios/*.sim                deterministic IO fixtures
tests/                         self-tests and simulator tests
```

## Commands

```sh
make test
make -f Makefile.linux test
make io-sim
scripts/run_with_result.sh smoke true
```

`make test` builds `out/io_sim`, runs shell harness self-tests, validates IO
scenario output formats, and checks JSON result output.

## IO Simulator

```sh
out/io_sim scenarios/basic_io.sim
out/io_sim --mqtt --site factory-a --node node-7 scenarios/basic_io.sim
out/io_sim --format jsonl scenarios/large_io.sim
```

Scenario commands operate at the raw driver boundary:

```text
channel, set, fault, stuck, noise, sleep, write, read,
expect, expect_between, expect_raw
```

This lets products test logic against the same raw state model used by real
GPIO, ADC, and field-bus drivers.

## CI Integration

Use `scripts/run_with_result.sh NAME COMMAND...` to wrap tests and emit JSON:

```json
{"name":"smoke","result":"pass","status":0,"duration_ms":0}
```

## TODO

TODO state is tracked in `docs/todo.yaml` and summarized in `docs/todo.md`.
