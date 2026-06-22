# Repository Guidelines

## Project Structure & Module Organization

`tools/io_sim.c` is the industrial IO scenario runner. Scenario fixtures live in
`scenarios/`. Reusable shell helpers live in `scripts/`, with assertions in
`scripts/assert.sh` and shared harness utilities in `scripts/testlib.sh`. Tests
live under `tests/`. Generated outputs belong in `out/` or `build_out/`.

## Build, Test, and Development Commands

- `make test` builds the IO simulator and runs the self-tests.
- `make -f Makefile.linux test` is the Linux-module-compatible alias.
- `make io-sim` builds `out/io_sim`.
- `scripts/run_with_result.sh NAME COMMAND...` runs a command and emits a JSON
  result summary.

## Scenario Conventions

Use `.sim` files for deterministic IO scenarios. Keep commands at the raw driver
boundary: channel definitions, raw writes, faults, stuck-at states, noise,
reads, expectations, and sleeps. Prefer explicit `expect` or `expect_between`
checks so product integrations can fail on behavior drift.

## Testing Guidelines

Add focused shell coverage under `tests/`. New fixture helpers should be tested
through public scripts or tools rather than private shell functions alone.

## TODO Workflow

Use `docs/todo.yaml` as the source of truth and keep `docs/todo.md` generated
from it. Set the relevant item to `in_progress` before changing behavior, then
mark it `done` and rerender the summary when complete.
