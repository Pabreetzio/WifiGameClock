# Wi‑Fi Game Clock

A modular, programmable game-clock platform for chess and any other game where time can be part of the rules.

The core idea is deliberately different from a traditional two-sided chess clock: **each player gets a small clock pod** with its own button, display, processor, and radio. Two pods can behave like an ordinary chess clock; four can run bughouse; more can support multiplayer board games, tournaments, team clocks, handicaps, experiments, and time controls that have not been invented yet.

The project began as an ESP8266 prototype and is being revived as an open hardware/software platform.

## The vision

A clock pod should be simple enough that a player can walk up and use it without a manual, but flexible enough that a developer can completely change how it behaves.

Each pod should eventually support:

- a large, satisfying physical player button
- an easy-to-read display, initially at least four 7-segment digits with a colon for `MM:SS`
- a wireless radio so clocks can discover and coordinate with one another
- groups of at least four clocks, with no architectural assumption that there are only two players
- configurable LEDs, button lighting, and sounds
- interchangeable physical controls: arcade buttons, clicky switches, piezo/touch controls, custom enclosures, etc.
- local configuration through a hosted web interface
- simple setup from a phone, eventually through a companion mobile app
- over-the-air firmware updates
- an open protocol and programmable firmware so unusual game rules do not require new hardware

The hardware should feel like a physical appliance. The complicated parts belong in the phone/web setup experience, not in memorizing long-presses and button combinations on the clock itself.

## Why multiple independent clocks?

Traditional chess clocks are built around two timers in one enclosure. That physical assumption becomes a software limitation.

Independent clock pods let the game define relationships between timers instead:

- **Chess:** pressing one player's button stops that clock and starts the opponent's.
- **Bughouse:** four players have four displays while each two-player team can share a pool of time.
- **Team games:** several players can debit the same team clock.
- **Multiplayer games:** turns can rotate through three or more players.
- **Handicaps:** players can have different starting time, increment, delay, or penalties.
- **Tournament play:** an organizer can configure clocks consistently without touching every device's physical controls.
- **Experimental controls:** time can be transferred, pooled, capped, accumulated, or modified by game events.

A useful mental model is that a pod is not "half a chess clock." It is a **networked timer endpoint** that can participate in a game-defined clock topology.

## Time-control model

The long-term goal is to represent time controls as data rather than hard-coded menu entries. A game configuration should describe:

1. **Participants** — players, teams, or other timer owners.
2. **Clocks** — independent countdown/count-up state.
3. **Bindings** — which physical pods display/control which clocks.
4. **Events** — button press, game start, pause, penalty, timeout, external command, etc.
5. **Rules** — what an event does to one or more clocks.

That model is what makes controls such as a shared bughouse team clock possible without treating bughouse as a one-off firmware hack.

See [`docs/time-controls.md`](docs/time-controls.md) for the initial design notes.

## Repository layout

This repository is becoming a monorepo for the complete platform. The existing prototype files are intentionally being preserved while the next generation is developed alongside them.

```text
WifiGameClock/
├── Firmware/              # Original ESP8266 prototypes (legacy, preserved)
├── Hardware/              # Original breadboard/Fritzing hardware work
├── Presentation/          # Historical project material
├── docs/                  # Product, protocol, architecture, and design notes
├── firmware/              # Next-generation embedded firmware
├── hardware/              # Next-generation schematics, PCB, enclosure work
├── apps/
│   ├── web/               # Browser-based clock setup/control UI
│   └── mobile/            # Companion mobile application
├── packages/              # Shared schemas/protocol/time-control libraries
└── site/                  # Public project/progress website for GitHub Pages
```

Lowercase directories are the new-generation workspace. The capitalized directories contain the historical prototype and remain useful reference material.

## First milestone

The first useful end-to-end version should prove the architecture with four physical pods:

1. Power on four clocks.
2. Discover them from a phone without typing IP addresses.
3. Assign each physical pod to a player/team by tapping the physical button.
4. Pick or create a time control in a clear graphical UI.
5. Start the game.
6. Run the clocks peer-to-peer or with an elected coordinator so gameplay does not depend on an internet connection.
7. Reconnect the phone at any time to inspect or adjust the game.
8. Update device firmware over the air.

The reference demo will be **four-player bughouse with shared team time**, because it exercises the capability that ordinary chess clocks cannot naturally provide.

## Design principles

- **Physical play must not depend on the cloud.** Internet services can add convenience, but a game should continue locally.
- **Setup should be discoverable.** Prefer phone/web UX over cryptic button menus.
- **The protocol is more important than one enclosure.** Custom hardware should be welcome.
- **Four players are a baseline, not an edge case.** Avoid two-player assumptions in the core model.
- **One source of truth for clock state.** Networking must tolerate retransmission, reconnects, and duplicate messages without corrupting time.
- **OTA is part of the product, not an afterthought.** Devices should be maintainable after they are assembled.
- **Hackable by design.** Schematics, firmware, protocols, and configuration formats should remain understandable and replaceable.

## Historical prototype

The original firmware already demonstrated several pieces of the concept using an ESP8266, TM1637 four-digit display, a physical button, a tiny hosted setup page, HTTP/WebSocket communication between clocks, Wi‑Fi station/AP fallback, and mDNS. That prototype stays in `Firmware/` as a useful proof of concept while the new architecture is built.

## Project status

**Revival / architecture phase.** The next work is to document the time-control/state model, choose the next hardware platform and local radio topology, and build the four-clock reference implementation.

Follow the design notes in [`docs/`](docs/) and the public progress site in [`site/`](site/).
