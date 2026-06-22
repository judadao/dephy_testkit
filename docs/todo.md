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

- [ ] Add assert_eventually with timeout and interval controls.
- [ ] Add MQTT subscriber capture helpers.
- [ ] Add broker build helpers that compile per-port binaries when needed.
- [ ] Add P2P mesh fixture that starts N static-seed brokers.
- [ ] Add MQTT bridge integration for the industrial IO simulator.
