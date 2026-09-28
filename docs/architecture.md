# Architecture direction

This is intentionally a direction document, not a final component selection.

## System layers

### 1. Clock pod

A pod is the reliable physical endpoint:

- microcontroller + radio
- four-or-more-digit time display
- primary player button
- optional RGB/status lighting
- optional sound
- power/battery management
- persistent device identity
- local clock rendering
- game-state synchronization
- OTA update support
- recovery/provisioning mode

The device must remain useful without an internet connection.

### 2. Local game network

Pods participating in one game form a local group. The group is responsible for:

- discovery
- membership
- role assignment
- authoritative game state
- ordered state transitions
- clock synchronization
- reconnect/recovery

Gameplay should not require a cloud round trip. A phone should be optional after setup.

### 3. Device web UI

At least one device can expose a small local web application/API for provisioning, diagnostics, firmware information, and basic setup. This is also the escape hatch when the mobile application is unavailable.

### 4. Companion app

The phone provides the rich configuration UX:

- discover pods
- identify a pod by pressing/flashing it
- create player/team topology
- select/edit time controls
- start/pause/administer games
- inspect battery/firmware/network health
- initiate updates

The app should describe game rules and send configuration; it should not need to remain connected to keep clocks accurate.

### 5. Optional internet services

Cloud functionality is optional and additive: firmware releases, saved presets, tournament management, game history, telemetry (opt-in), and remote administration. Loss of internet should not stop an in-progress local game.

## Hardware direction

The original prototype used an ESP8266 and TM1637 four-digit display. The revival should evaluate a modern MCU/radio platform against these requirements:

- inexpensive enough to put one in every player pod
- Wi‑Fi for web UI and OTA
- a low-friction local peer transport
- BLE if useful for phone provisioning
- enough flash/RAM for signed OTA and a small web UI
- low-power modes for battery operation
- strong ecosystem and long-term availability

An ESP32-family part is an obvious candidate, but the project should record the decision rather than silently locking itself to one vendor.

## Networking principles

1. **Separate discovery from gameplay.** Discovery can be chatty/convenient; gameplay messages should be small and deterministic.
2. **Transmit state transitions, not display ticks.** Every pod renders time locally.
3. **Idempotent commands.** Retransmitting a packet must not add an increment twice.
4. **Version game state.** Ignore stale transitions.
5. **Use monotonic time for elapsed-time calculations.** Wall-clock time is metadata, not the countdown source.
6. **Design for coordinator loss.** A dead/rebooted pod should not necessarily destroy the whole game.
7. **Internet independence.** The local group owns the live game.

## Monorepo target

```text
firmware/
  pod/                 Embedded application
  bootloader/          Only if a custom layer becomes necessary
hardware/
  reference-pod/       Schematics/PCB/BOM
  enclosure/           Printable/mechanical designs
apps/
  web/                  Local/PWA configuration interface
  mobile/               Native/cross-platform companion
packages/
  protocol/             Wire messages and generated types
  time-control/         Schemas, validation, simulation
  ui/                   Optional shared UI pieces
docs/
  architecture.md
  time-controls.md
  hardware-decisions.md
  protocol.md
site/
  index.html             Project/progress site initially hosted by GitHub Pages
```

## Suggested first engineering spike

Build four development nodes before designing a polished enclosure. Each node only needs:

- candidate MCU dev board
- 4-digit display
- one large button
- one RGB/status LED
- USB power

Then prove, in order:

1. stable monotonic countdown locally
2. button debounce and deterministic transitions
3. peer discovery
4. two-clock chess
5. four physical clocks
6. logical clocks independent from physical pods
7. shared-team bughouse control
8. phone-based assignment/setup
9. reconnect and coordinator failure
10. OTA update

A custom PCB and industrial design make much more sense once those boundaries are stable.
