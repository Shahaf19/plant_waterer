# plant_waterer

An automatic plant watering system on an Arduino Uno. It reads a capacitive soil moisture
sensor and waters the plant in small, timed doses through a relay-switched pump.

> **Status:** firmware and unit tests complete; waiting on hardware calibration and on-board checks.

## Safety by design


- **Fixed doses with soak time.** The pump runs for a few seconds, then waits for the water to
  reach the sensor before measuring again. This prevents overwatering caused by sensor lag.
- **Hard limits.** Each dose is time-limited, and too many doses in a row (empty tank, stuck
  sensor) trigger a fault.
- **Fail-safe.** The pump is off at boot and in any fault; a fault stays until a person resets.
- **Hysteresis.** Watering starts below one moisture level and stops above a higher one, so the
  pump never toggles rapidly around a single threshold.

## Architecture

The watering logic is plain C++ with no Arduino dependencies, so it is unit-tested on a PC.
Hardware access sits in thin wrappers at the edges.

```mermaid
flowchart TD
    Soil(["🌱 Soil"])
    Read["① Read the sensor"]
    Convert["② Convert to % and check the sensor works"]
    Clock(["⏱ Clock"])
    Decide{"③ Decide what to do"}
    Pump["④ Switch the pump on / off"]
    Report["⑤ Report status to the PC"]
    Water(["💧 Water"])

    Soil -.-> Read
    Read -->|"raw number 0–1023"| Convert
    Convert -->|"moisture % or 'sensor broken'"| Decide
    Clock -.->|"time now"| Decide
    Decide -->|"pump on?"| Pump
    Decide -->|"current state"| Report
    Pump -.-> Water -.-> Soil

    classDef world fill:#eeeeee,stroke:#999,color:#333
    classDef hardware fill:#4fc3f7,stroke:#333,color:#000
    classDef logic fill:#00cc88,stroke:#333,color:#000
    class Soil,Clock,Water world
    class Read,Pump,Report hardware
    class Convert,Decide logic
```

🟩 Green = pure logic, unit-tested on the PC · 🟦 Blue = hardware, checked on the board

More detail: [design](docs/design.md) (components, state machine).

## Hardware

Arduino Uno (USB power) · capacitive soil moisture sensor on **A0** · 1-channel relay module
(active-LOW) on **D10** · small water pump on its own 3.7 V 18650 battery, switched by the relay.

## Configuration

All tunables live in [`include/config.h`](include/config.h). Pick the plant with one line,
e.g. `ACTIVE_PLANT = MINT` (presets: cactus, herbs, lemongrass, mint, fern).

## Build, test, run

Requires [PlatformIO](https://platformio.org/).

```bash
pio test -e native            # run the unit tests on the PC
pio run -e uno                # build firmware
pio run -e uno -t upload      # flash the board
pio device monitor            # watch the status output
```
