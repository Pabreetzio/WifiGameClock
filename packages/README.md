# Shared packages

This workspace is intended for code/data definitions shared across firmware, web, mobile, simulators, and tooling.

Likely packages:

- `protocol` — message/schema definitions and compatibility rules
- `time-control` — time-control schema, validation, state-machine simulation, and test vectors

The time-control engine should be testable on a desktop without physical hardware. A simulated four-player bughouse game is a useful acceptance target before embedding the same semantics in clock firmware.
