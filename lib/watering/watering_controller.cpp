#include "watering_controller.h"

WateringController::WateringController(WateringSettings wateringSettings) {
    settings_ = wateringSettings;
    state_ = State::Monitoring;  // fail-safe start: pump off
    dosesInRow_ = 0;
    stateSinceMs_ = 0;
}

void WateringController::update(MoistureReading reading, uint32_t nowMs) {
    // Latched: only a reset (which re-creates the controller) clears a fault.
    if (state_ == State::Fault) {
        return;
    }
    if (!reading.valid) {
        enterState(State::Fault, nowMs);
        return;
    }

    // Unsigned subtraction stays correct when millis() wraps around after ~49.7 days.
    const uint32_t elapsedMs = nowMs - stateSinceMs_;

    switch (state_) {
        case State::Monitoring:
            if (reading.percent < settings_.startBelowPercent) {
                enterState(State::Dosing, nowMs);
            }
            break;

        case State::Dosing:
            // Ends on time alone, never on the sensor, so the pump can't run unbounded.
            if (elapsedMs >= settings_.doseMs) {
                dosesInRow_++;
                enterState(State::Soaking, nowMs);
            }
            break;

        case State::Soaking:
            // The water needs time to reach the sensor; until then the reading still looks dry.
            if (elapsedMs < settings_.soakMs) {
                break;
            }
            if (reading.percent >= settings_.stopAtPercent) {
                dosesInRow_ = 0;
                enterState(State::Monitoring, nowMs);
            } else if (dosesInRow_ >= settings_.maxDosesInRow) {
                // Still dry after every allowed dose: empty tank or stuck sensor.
                enterState(State::Fault, nowMs);
            } else {
                enterState(State::Dosing, nowMs);
            }
            break;

        case State::Fault:
            break;  // handled at the top; listed so the compiler sees every state covered
    }
}

bool WateringController::isPumpOn() const {
    return state_ == State::Dosing;
}

WateringController::State WateringController::state() const {
    return state_;
}

void WateringController::enterState(State state, uint32_t nowMs) {
    state_ = state;
    stateSinceMs_ = nowMs;
}
