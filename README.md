# dephy_testkit

Reusable Linux test harness and IO simulator fixtures for Dephy repos.

`dephy_testkit` collects the test utilities that otherwise get copied from repo
to repo: shell assertions, cleanup helpers, broker fixtures, P2P fixtures,
machine-readable result wrappers, and deterministic industrial IO scenarios.

## Overview

Use this repo when a module or product needs repeatable Linux tests. It provides
the shared test building blocks so each repo can focus on behavior under test
instead of rewriting cleanup, retries, fixtures, and IO simulation.

## Key Value

- Routine integration tests should be cheap to add.
- Products and modules should share the same process cleanup and retry helpers.
- IO behavior can be tested from scenario files before hardware is available.
- CI can consume JSON result output instead of scraping ad hoc logs.

## How To Use

1. Source `scripts/assert.sh` or `scripts/testlib.sh` from a repo test.
2. Use fixture helpers to start brokers, allocate ports, and clean processes.
3. Wrap external commands with `scripts/run_with_result.sh` when JSON output is
   useful.
4. Use `out/io_sim` with `.sim` scenario files to test IO-dependent product
   logic on Linux.

## How It Works

The shell harness owns boring but important behavior: temporary directories,
port allocation, eventual assertions, cleanup traps, and consistent pass/fail
reporting. The IO simulator executes scenario commands against the same raw
state concepts used by `dephy_industrial_io`: channel values, faults, stuck
states, noise, reads, writes, and expectations.

Because scenarios operate at the driver boundary, a product can validate logic
against realistic IO transitions without requiring a connected ESP32 or field
device.

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

IO simulator examples:

```sh
out/io_sim scenarios/basic_io.sim
out/io_sim --mqtt --site factory-a --node node-7 scenarios/basic_io.sim
out/io_sim --format jsonl scenarios/large_io.sim
```

## TODO

TODO state is tracked in `docs/todo.yaml` and summarized in `docs/todo.md`.
