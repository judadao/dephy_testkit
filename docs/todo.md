# TODO

Source of truth: `docs/todo.yaml`. Update YAML before starting or completing work.

## io_sim

- [x] Allow scenario files to declare IO channel maps.
- [x] Validate scenario expectations inside io_sim.
- [x] Emit MQTT-like topic/payload lines for product integration tests.
- [x] Add configurable output formats for text, JSONL, and MQTT command streams.
- [x] Add scenario examples for DI, DO, AI, AO, fault, stuck-at, and noise states.
- [x] Keep simulator commands operating at the raw driver boundary used by real drivers.

## harness

- [x] Add assert_eventually with timeout and interval controls.
- [x] Add MQTT subscriber capture helpers.
- [x] Add broker build helpers that compile per-port binaries when needed.
- [x] Add P2P mesh fixture that starts N static-seed brokers.
- [x] Add MQTT bridge integration for the industrial IO simulator.
- [ ] Add machine-readable test result summaries for product CI aggregation.

## repo

- [ ] Align repository layout with dephy_module_golden_sample. (`in_progress`)
- [ ] Add AGENTS.md with harness workflow, scenario conventions, and validation commands.
- [ ] Add docs/module_structure.md describing scripts, tools, fixtures, scenarios, and tests.
- [ ] Add Makefile.linux or document the Makefile compatibility path expected by module repos.

## performance

- [ ] Add a large IO scenario regression to measure simulator runtime and fixture startup overhead.
