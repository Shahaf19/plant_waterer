#pragma once
#include <stdint.h>
#include "moisture.h"

struct WateringSettings {
    uint8_t startBelowPercent;
    uint8_t stopAtPercent;
    uint32_t doseMs;
    uint32_t soakMs;
    uint8_t maxDosesInRow;
};

class WateringController {
    public:
    enum class State : uint8_t {Monitoring, Dosing, Soaking, Fault};

    WateringController(WateringSettings wateringSettings);

    // The only function that changes the controller; call it on every loop pass.
    void update(MoistureReading reading, uint32_t nowMs);

    // True exactly while dosing, so the pump can never disagree with the state.
    bool isPumpOn() const;
    State state() const;

    private:
    void enterState(State state, uint32_t nowMs);
    WateringSettings settings_;
    State state_;
    uint32_t stateSinceMs_;
    uint8_t dosesInRow_;
};
