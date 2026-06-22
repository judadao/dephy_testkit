# dephy_testkit Module Structure

`dephy_testkit` provides reusable Linux test harness utilities and IO simulator
fixtures for Dephy product and module repositories.

## Tools

- `tools/io_sim.c`: scenario-driven industrial IO simulator built against
  `dephy_industrial_io`.

## Scripts

- `scripts/assert.sh`: shell assertion helpers.
- `scripts/testlib.sh`: shared test lifecycle, cleanup, port allocation, and
  eventually assertions.
- `scripts/broker_fixture.sh`: broker build/start helpers.
- `scripts/p2p_mesh_fixture.sh`: static-seed P2P broker mesh helpers.
- `scripts/run_with_result.sh`: command wrapper that emits JSON result summaries
  for CI aggregation.

## Scenarios

- `scenarios/basic_io.sim`: default DI/DO/AI workflow.
- `scenarios/channel_map.sim`: explicit channel map coverage.
- `scenarios/fault_io.sim`: fault and stuck-at raw-state coverage.
- `scenarios/noise_io.sim`: raw noise/jitter coverage.
- `scenarios/large_io.sim`: larger channel map regression for simulator runtime
  and output stability.

## Tests

- `tests/selftest.sh`: shell harness self-test.
- `tests/test_io_sim.sh`: simulator scenario and output format coverage.
- `tests/test_result_json.sh`: machine-readable test result coverage.

## Integration Rule

Product repos should use testkit scripts and fixtures instead of copying shell
test harness code. Reusable helpers belong here first, then products consume a
tagged version.
