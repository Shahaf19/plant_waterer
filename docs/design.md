# Design

An Arduino Uno reads a soil moisture sensor and waters the plant in small doses through a
relay-switched pump. Watering logic is plain C++ with no `Arduino.h`, so it is unit-tested on the PC.

## Architecture

One `loop()` pass moves data left to right. Logic never calls hardware; only `main.cpp` knows
every part.

```mermaid
flowchart LR
    S[SoilSensor<br/><i>hardware</i>] -- raw 0–1023 --> M[Moisture<br/><i>pure logic</i>]
    M -- "% or bad reading" --> C[WateringController<br/><i>pure logic</i>]
    T((millis)) -- now --> C
    C -- on/off --> P[Pump<br/><i>hardware</i>]
    C -- state, % --> L[StatusLogger<br/><i>hardware: serial</i>]
```

## Components

Green = pure logic, unit-tested on the PC. Blue = hardware, verified on the board.

```mermaid
classDiagram
    direction TB

    class MainLoop {
        <<main.cpp>>
        +setup() void
        +loop() void
    }

    class Config {
        <<config.h>>
        PIN_SENSOR, PIN_RELAY
        SENSOR_RAW_DRY ~600, SENSOR_RAW_WET ~300
        MOISTURE_ON_PERCENT = 40
        MOISTURE_OFF_PERCENT = 60
        DOSE_MS = 5000
        SOAK_MS = 600000
        MAX_DOSES_IN_ROW = 3
        RELAY_ACTIVE_LOW
    }

    class SoilSensor {
        -pin : uint8_t
        +readRaw() uint16_t
    }

    class Moisture {
        <<pure functions>>
        +fromRaw(raw: uint16_t) MoistureReading
    }

    class MoistureReading {
        <<struct>>
        +valid : bool
        +percent : uint8_t
    }

    class WateringController {
        -state : State
        -stateSinceMs : uint32_t
        -dosesInRow : uint8_t
        +update(reading: MoistureReading, nowMs: uint32_t) void
        +isPumpOn() bool
        +state() State
    }

    class State {
        <<enumeration>>
        MONITORING
        DOSING
        SOAKING
        FAULT
    }

    class Pump {
        -pin : uint8_t
        +begin() void
        +set(on: bool) void
    }

    class StatusLogger {
        -lastState : State
        +logIfChanged(state: State, percent: uint8_t) void
    }

    MainLoop --> SoilSensor : readRaw()
    MainLoop --> Moisture : fromRaw()
    MainLoop --> WateringController : update()
    MainLoop --> Pump : set()
    MainLoop --> StatusLogger : logIfChanged()
    Moisture ..> MoistureReading : creates
    WateringController ..> MoistureReading : reads
    WateringController *-- State

    style MainLoop fill:#fdd835,stroke:#333
    style Config fill:#e0e0e0,stroke:#333
    style SoilSensor fill:#4fc3f7,stroke:#333
    style Pump fill:#4fc3f7,stroke:#333
    style StatusLogger fill:#4fc3f7,stroke:#333
    style Moisture fill:#00cc88,stroke:#333
    style MoistureReading fill:#00cc88,stroke:#333
    style WateringController fill:#00cc88,stroke:#333
    style State fill:#00cc88,stroke:#333
```

| Component | Responsibility |
|-----------|----------------|
| `Config` | Every tunable number (pins, calibration, thresholds, timings) in one place |
| `SoilSensor` | Read the raw ADC value from the sensor pin |
| `Moisture` | Raw value → 0–100 % (clamped); flag readings outside the calibrated range |
| `WateringController` | The state machine below; time comes in as a parameter, so tests control it |
| `Pump` | Drive the relay; starts OFF at boot; hides whether the relay is active-LOW |
| `StatusLogger` | Print a line over serial when the state changes |
| `MainLoop` | Wire the parts together, once per `loop()` pass |

## State machine

Watering is a fixed dose, then a soak, then a fresh measurement (see decision #7).

```mermaid
stateDiagram-v2
    [*] --> Monitoring : power on / reset
    Monitoring --> Dosing : moisture below 40%
    Dosing --> Soaking : 5 s passed, doses + 1
    Soaking --> Monitoring : 10 min passed, at least 60%, doses = 0
    Soaking --> Dosing : 10 min passed, below 60%, fewer than 3 doses
    Soaking --> Fault : 10 min passed, below 60%, 3 doses given
    Monitoring --> Fault : bad reading
    Dosing --> Fault : bad reading
    Soaking --> Fault : bad reading
    Fault --> [*] : only the reset button
```

| State | Pump | Leaves when |
|-------|------|-------------|
| Monitoring | off | Moisture below 40 % → Dosing |
| Dosing | **on** | 5 s passed → Soaking (doses + 1) |
| Soaking | off | After 10 min: at least 60 % → Monitoring (doses = 0); below 60 % → Dosing, or Fault after 3 doses |
| Fault | off | Never by itself; reset restarts in Monitoring (decision #8) |
| *any* | — | Bad sensor reading → Fault |

All numbers are placeholders from `Config`; calibration (Milestone 1) sets the real ones.
