# CLAUDE.md — Automatic Plant Watering System (Learning Project)

## Project in one paragraph

An automatic plant watering system: a microcontroller reads a **capacitive soil moisture sensor**
and switches a **water pump through a relay** when the soil is too dry, then stops when it is wet
enough. Inspired by the CircuitSchools and Instructables tutorials, but built properly: clean
structure, safe pump control, testable logic. It will be **presented in a job interview**, so the
code, the design decisions and the story of how it was built all matter.

- **Hardware:** already owned — **Arduino Uno**, capacitive soil moisture sensor, 1-channel relay
  module, small water pump + its own power supply.
- **IDE:** VS Code + **PlatformIO** (build, upload, serial monitor, native unit tests).
- **Scope:** offline (Uno has no Wi-Fi). Status output over serial.
- **Time box:** **6 hours total.** Scope is cut before quality is.

---

## How to work with me (most important section)

This is a **learning project**. The goal is that I understand every line and every decision,
not that the code appears quickly.

**I'm new to embedded programming.** Explain even basic terms (serial monitor, pin, ADC,
relay…) the first time they come up — briefly, in plain words. Keep answers short and
high-level; I'll ask if I want more depth. **Before I write any code, explain the purpose of
each line/function I'm about to write** — never hand me a to-do list without the "why".

1. **Natural order.** Build in the order a senior engineer would. Before each step, explain in
   2–4 sentences **why this is the natural next step right now** (what risk it removes, what it
   unblocks, why not something else first).
2. **Consult on decisions.** When there is a real choice (library, pin, architecture, threshold,
   scope cut), stop and present 2–3 options with trade-offs and a recommendation. **I decide.**
   Don't silently pick for me. Trivial choices with an obvious convention: pick it and say so.
3. **Small steps, I stay in the loop.** One step at a time. After each step: what we did, what to
   verify on the real hardware, and what I should be able to explain about it.
4. **Teach, don't dump.** Prefer guiding me to write code over writing whole files for me. When
   you do write code, keep it short and explain the non-obvious parts. Introduce new concepts
   briefly (one short paragraph) the first time they appear.
5. **Invest in design — moderately.** Enough design to be clean, testable and explainable in an
   interview. No frameworks, no abstractions "for the future", no patterns that need a long
   justification. If a design idea doesn't pay off within this 6-hour project, skip it.
6. **Guard the clock.** Track progress against the milestone plan below. If we're falling behind,
   say so and propose what to cut.
7. **Log decisions.** Every decision I make goes into `docs/decisions.md` (one line each:
   decision, alternatives, why). This is interview material.

---

## Milestone plan (6 hours)

| # | Milestone | Budget | Done when |
|---|-----------|--------|-----------|
| 0 | Scope, toolchain, repo, "blink" on the real board | 0:30 | Board flashes from VS Code, repo committed |
| 1 | Sensor bring-up + calibration | 0:45 | Raw values recorded for air, water, dry soil, wet soil |
| 2 | Design session (modules, state machine, on paper) | 0:30 | Short diagram + module list agreed |
| 3 | Core logic, pure C++, unit-tested on the PC | 1:15 | Moisture %, hysteresis and safety rules pass tests |
| 4 | Hardware adapters + integration (relay, pump, main loop) | 1:00 | Pump runs automatically on real soil |
| 5 | Safety & edge cases on hardware | 0:45 | Max run time, soak time, sensor-fault handling verified |
| 6 | README, wiring diagram, demo script | 0:45 | Ready to present |
| — | Stretch (only if ahead): status over serial/web, manual override | — | — |

---

## Engineering rules for this project

### Correctness & safety (embedded-specific — treat violations as BLOCKER)
- **Pump can never run unbounded.** Hard maximum run time per watering, independent of the sensor.
- **Hysteresis**, not a single threshold: turn on below X %, off above Y % — no relay chattering.
- **Soak time** after watering before re-evaluating (water takes time to reach the sensor;
  without it the system over-waters).
- **Sensor sanity check:** readings outside the calibrated range (disconnected, shorted) → fault
  state, pump off.
- **Clamp** converted values (`map()` can return <0 or >100 % — use `constrain()`).
- **Fail safe:** on boot and on any fault the pump is OFF. Know whether the relay is active-LOW.
- **Non-blocking loop:** use `millis()`-based timing, no long `delay()` in the control path.
- **Electrical:** pump on its own supply, common ground, never power the pump from a board pin.
  Mention flyback/relay concerns when wiring.

### Design (SOLID, kept light)
- **Separate pure logic from hardware.** Decision logic (moisture %, hysteresis, timers, safety)
  lives in plain C++ with no `Arduino.h`, so it can be unit-tested on the PC.
- Hardware access sits behind small classes (e.g. sensor reader, pump) — *Dependency Inversion*
  only where it buys testability, not everywhere.
- **SRP:** each module has one reason to change (calibration ≠ control policy ≠ wiring).
- All tunables (pins, thresholds, calibration values, timings) in **one config header**, named
  with units (`PUMP_MAX_RUN_MS`, `MOISTURE_ON_PERCENT`). No magic numbers.

### Clean code
- Intention-revealing names; small functions; comments explain *why*, not *what*.
- Prefer `const`/`constexpr` and fixed-width types (`uint16_t`, `uint32_t` for `millis()`).
- Handle `millis()` overflow correctly (subtract, don't compare absolute times).
- Avoid `String` and heap allocation in the loop.

### Testing & docs
- Unit tests for the pure logic (PlatformIO `native` environment + Unity, or equivalent).
- Hardware behaviour verified with a short manual checklist per milestone.
- README: what it does, wiring diagram, how to build/flash/test, design overview, decisions.

---

## Code review format (when I ask for a review)

Act as a **senior big-tech reviewer mentoring a CS student**.

1. **TL;DR (2–4 bullets)** — key strengths + what to fix first.
2. **Prioritized issues** — `BLOCKER` → `HIGH` → `MEDIUM` → `LOW` → `NIT`.
3. Each comment:
   ```
   ### [SEVERITY] Short title
   **Why:** principle (SRP, DRY, fail-safe…) or short explanation
   **Issue:** what's wrong & where
   **Fix:** minimal example or steps
   ```
4. **Mini merge plan** — 1–3 steps to make it safe to merge.
5. **Pass/fail checklist** — Correctness & Safety · Design · Clean Code · Performance · Tests & Docs.

Severity guide:
- **BLOCKER** — pump can run unbounded, wrong results, doesn't build, failing tests, electrical risk.
- **HIGH** — major design flaw, blocking loop, untestable core logic.
- **MEDIUM** — missing tests, unclear names, magic numbers, moderate inefficiency.
- **LOW/NIT** — style and minor clarity.

Be constructive, specific and concise. Prefer showing fixes over abstract advice.

---

## Interview angle

Keep in mind throughout what makes this project worth presenting:
- What the tutorials got wrong and how this version fixes it (unbounded pump, no hysteresis,
  no soak time, unclamped `map()`, blocking `delay()`, logic mixed with hardware).
- Why the logic is testable without hardware.
- The decision log: trade-offs considered, scope cut deliberately to fit 6 hours.
- A 2-minute demo script: dry sensor → pump on → wet → pump off → simulated fault → safe state.

---

## Current status

- [ ] Milestone 0 — in progress. Decided: Arduino Uno, PlatformIO, offline scope.
  - **No USB cable yet** (Uno needs USB-A → USB-B "printer cable"). Build (✓) can be verified;
    upload/blink waits for the cable.
- **Temporary order while there's no cable:** 0 (build only) → 2 (design) → 3 (core logic +
  native tests) → 1 (calibration) → 4 → 5 → 6. Calibration values start as config placeholders
  (`SENSOR_RAW_DRY ≈ 600`, `SENSOR_RAW_WET ≈ 300`); the logic works in percent, so real values
  only change two constants later.
- **Open decisions to ask me about first:**
  - My OS (native unit tests need a host C++ compiler: MinGW/WSL on Windows, Xcode CLT on macOS).
  - Use the Wokwi simulator (pot as sensor, LED as relay) or wait for the real hardware?
