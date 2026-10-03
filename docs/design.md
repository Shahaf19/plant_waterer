# Design

An Arduino Uno reads a soil moisture sensor and waters the plant in small doses through a
relay-switched pump. Watering logic is plain C++ with no `Arduino.h`, so it is unit-tested on the PC.

## Architecture

What happens in one pass of the main loop (it repeats non-stop). Logic never touches hardware;
only `main.cpp` connects the steps.

```mermaid
flowchart TD
    Soil(["🌱 Soil"])
    Read["① Read the sensor<br/><i>SoilSensor</i>"]
    Convert["② Convert to % and check the sensor works<br/><i>moistureFromRaw</i>"]
    Clock(["⏱ Clock"])
    Decide{"③ Decide what to do<br/><i>WateringController</i>"}
    Pump["④ Switch the pump on / off<br/><i>Pump → relay</i>"]
    Report["⑤ Report status to the PC<br/><i>StatusLogger → USB</i>"]
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

🟩 Green = pure logic, unit-tested on the PC · 🟦 Blue = hardware, checked on the board ·
⬜ Grey = the physical world

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
        PIN_SENSOR = A0, PIN_RELAY = 10
        RELAY_ACTIVE_LOW = true
        SENSOR_RAW_DRY ~600, SENSOR_RAW_WET ~300
        SENSOR_MARGIN_RAW = 50
        START_BELOW_PERCENT = 40
        STOP_AT_PERCENT = 60
        DOSE_MS = 5000
        SOAK_MS = 600000
        MAX_DOSES_IN_ROW = 3
    }

    class SoilSensor {
        -pin_ : uint8_t
        +SoilSensor(pin: uint8_t)
        +readRaw() uint16_t
    }

    class Moisture {
        <<moisture.h>>
        +moistureFromRaw(raw: uint16_t, calibration: SensorCalibration) MoistureReading
    }

    class SensorCalibration {
        <<struct>>
        +rawDry : uint16_t
        +rawWet : uint16_t
        +marginRaw : uint16_t
    }

    class MoistureReading {
        <<struct>>
        +valid : bool
        +percent : uint8_t
    }

    class WateringSettings {
        <<struct>>
        +startBelowPercent : uint8_t
        +stopAtPercent : uint8_t
        +doseMs : uint32_t
        +soakMs : uint32_t
        +maxDosesInRow : uint8_t
    }

    class WateringController {
        -settings_ : WateringSettings
        -state_ : State
        -stateSinceMs_ : uint32_t
        -dosesInRow_ : uint8_t
        +WateringController(settings: WateringSettings)
        +update(reading: MoistureReading, nowMs: uint32_t) void
        +isPumpOn() bool
        +state() State
        -enterState(state: State, nowMs: uint32_t) void
    }

    class State {
        <<enumeration>>
        Monitoring
        Dosing
        Soaking
        Fault
    }

    class Pump {
        -pin_ : uint8_t
        -activeLow_ : bool
        +Pump(pin: uint8_t, activeLow: bool)
        +begin() void
        +set(on: bool) void
    }

    class StatusLogger {
        -lastState_ : State
        +logIfChanged(state: State, percent: uint8_t) void
    }

    MainLoop --> SoilSensor : readRaw()
    MainLoop --> Moisture : moistureFromRaw()
    MainLoop --> WateringController : update()
    MainLoop --> Pump : set()
    MainLoop --> StatusLogger : logIfChanged()
    Moisture ..> SensorCalibration : reads
    Moisture ..> MoistureReading : creates
    WateringController ..> MoistureReading : reads
    WateringController *-- WateringSettings
    WateringController *-- State

    style MainLoop fill:#fdd835,stroke:#333
    style Config fill:#e0e0e0,stroke:#333
    style SoilSensor fill:#4fc3f7,stroke:#333
    style Pump fill:#4fc3f7,stroke:#333
    style StatusLogger fill:#4fc3f7,stroke:#333
    style Moisture fill:#00cc88,stroke:#333
    style SensorCalibration fill:#00cc88,stroke:#333
    style MoistureReading fill:#00cc88,stroke:#333
    style WateringSettings fill:#00cc88,stroke:#333
    style WateringController fill:#00cc88,stroke:#333
    style State fill:#00cc88,stroke:#333
```

| Component | Responsibility |
|-----------|----------------|
| `Config` | Every tunable number (pins, calibration, thresholds, timings) in one place |
| `SoilSensor` | Read the raw ADC value from the sensor pin |
| `moistureFromRaw` | Raw value → 0–100 % (clamped); flag readings outside the calibrated range |
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
