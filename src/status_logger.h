#pragma once

#include <stdint.h>

#include "moisture.h"
#include "watering_controller.h"

// Reports what the system is doing over USB serial: every state change immediately,
// plus a status line (state, moisture, raw value) at a fixed interval.
class StatusLogger {
    public:
    StatusLogger(uint32_t statusIntervalMs);

    // Call once from setup(): opens the serial link.
    void begin(uint32_t baud);

    // Call on every loop pass; decides by itself whether anything is worth printing.
    void report(WateringController::State state, MoistureReading reading, uint16_t raw, uint32_t nowMs);

    private:
    uint32_t statusIntervalMs_;
    uint32_t lastStatusMs_;
    WateringController::State lastState_;
};
