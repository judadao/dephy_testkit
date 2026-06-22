# dephy_testkit

Reusable Linux test harness and IO simulator fixtures for Dephy repos.

## Overview

`dephy_testkit` provides shared test utilities: assertions, cleanup, broker
fixtures, P2P fixtures, JSON result wrapping, and deterministic IO scenarios.

## Key Value

- Faster integration test authoring.
- Shared process cleanup, retries, and port helpers.
- IO scenario tests without ESP32 hardware.
- Machine-readable JSON test results.

## How To Use

```sh
make test
make io-sim
scripts/run_with_result.sh smoke true
out/io_sim scenarios/basic_io.sim
out/io_sim --format jsonl scenarios/large_io.sim
```

## Simple Principle

Tests should describe behavior, not reimplement fixtures. The IO simulator works
at the raw state boundary used by real IO drivers.

## Docs

- `docs/todo.md`: current TODO summary.
