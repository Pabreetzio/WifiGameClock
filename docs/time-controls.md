# Time-control model

This document is a starting point for designing time controls without baking chess-specific assumptions into the firmware.

## Goal

A physical clock pod should know how to display time, accept local input, communicate, and execute a synchronized game state. It should not require the firmware to contain a giant menu of named chess modes.

A time control is better represented as a small state machine/configuration shared by all devices participating in a game.

## Concepts

### Pod

A physical device. A pod has an identity and capabilities such as:

- display type
- button/input types
- LEDs
- speaker/buzzer
- battery state
- firmware version
- supported protocol version

Pods are assigned to roles when a game is configured. Hardware identity and player identity are separate.

### Participant

A human or logical participant in a game. Usually a player, but it may also represent a team or another game-specific role.

### Clock

A logical source of time. A clock is not necessarily owned by exactly one pod or one player.

Examples:

- standard chess: one clock per player
- shared-time bughouse: one clock per team, displayed on both teammates' pods
- hybrid bughouse: individual player clocks plus a team reserve

### Binding

Maps physical pods and participants to logical clocks. Keeping this explicit lets a player choose a pod/button without changing the game rules.

### Event

Something that may change game state. Initial event vocabulary:

- `game.start`
- `game.pause`
- `game.resume`
- `game.reset`
- `pod.press`
- `clock.timeout`
- `clock.adjust`
- `penalty.apply`

The vocabulary can grow without requiring every game to use every event.

### Rule / transition

A deterministic reaction to an event. Examples:

- stop clock A
- start clock B
- add 2 seconds to A
- subtract 30 seconds from team B
- advance active participant
- end the game

## Example: ordinary chess

Two pods, two players, two logical clocks.

```text
White pod --press--> stop White clock, start Black clock
Black pod --press--> stop Black clock, start White clock
```

The familiar behavior emerges from configuration rather than from a special two-sided physical enclosure.

## Example: shared-time bughouse

Four physical pods:

```text
Board 1: Alice (Team A) vs Bob   (Team B)
Board 2: Carol (Team A) vs David (Team B)
```

There are only two logical countdown pools:

```text
Team A: 10:00
Team B: 10:00
```

Alice and Carol both display Team A's remaining time. Bob and David both display Team B's remaining time.

The interesting design question is **when a team's shared clock runs**. Possible controls include:

1. Team time runs whenever either teammate is on move.
2. Team time runs at normal speed with one active teammate and faster when both are on move.
3. A team owns a shared reserve that individual board clocks draw from according to configurable rules.

The platform should be able to express these as configurations rather than selecting one interpretation forever in firmware.

## Timing semantics

Clock correctness needs an authoritative timestamp/state model rather than devices repeatedly sending "subtract one second" messages.

A useful state representation is approximately:

```text
clock_id
remaining_ms_at_epoch
running_since_monotonic_timestamp (optional)
rate
sequence/version
```

Each pod can render the countdown locally between synchronization messages. State-changing commands carry a monotonically increasing game-state version so duplicate or stale network packets cannot move the game backward.

The exact synchronization/election protocol is still TBD.

## Time-control building blocks

The first schema should be able to express at least:

- initial time
- Fischer increment
- Bronstein/simple delay
- per-move limit
- multiple periods/stages
- time additions/subtractions
- shared clocks
- multiple simultaneously active clocks
- clock rates other than 1x
- pause/resume
- timeout behavior

More exotic rules should ideally be compositions of these primitives rather than new firmware modes.

## Setup UX

Configuration should happen on a phone or browser, not through a tiny display menu.

A target setup flow:

1. Create a game.
2. Nearby unassigned pods appear visually.
3. Tap "Player 1" in the app and press the physical pod that player wants to use.
4. Repeat for the remaining players.
5. Choose a preset such as `5+0`, `3+2`, or `Bughouse — shared 10 minutes`.
6. Optionally open an advanced editor to customize the underlying control.
7. Preview the relationships between players, teams, and clocks.
8. Start.

The common path should require no knowledge of IP addresses, networking, or firmware terminology.

## Open questions

- Which node is authoritative: elected coordinator, replicated state machine, or another model?
- Which local transport best fits the hardware: normal Wi‑Fi, ESP-NOW, BLE, Thread, or a combination?
- How should a phone bootstrap a group onto Wi‑Fi without making Wi‑Fi mandatory for gameplay?
- How much of the time-control rule engine belongs on every pod versus a coordinator?
- What should happen if the coordinator disappears mid-game?
- How do we represent simultaneous button presses and network latency fairly?
- Should configurations be JSON/CBOR data, a constrained rule language, or compiled state machines?

These should be answered by the four-pod bughouse reference implementation rather than by prematurely optimizing for every possible game.
