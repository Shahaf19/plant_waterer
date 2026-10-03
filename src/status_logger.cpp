#include "status_logger.h"

#include <Arduino.h>

using State = WateringController::State;

// F("...") keeps the text in flash (32 KB) instead of copying it into RAM (2 KB) at boot.
static const __FlashStringHelper* stateName(State state) {
    switch (state) {
        case State::Monitoring: return F("Monitoring");
        case State::Dosing:     return F("Dosing");
        case State::Soaking:    return F("Soaking");
        case State::Fault:      return F("FAULT");
    }
    return F("?");
}

static void printTimestamp(uint32_t nowMs) {
    Serial.print('[');
    Serial.print(nowMs / 1000);
    Serial.print(F(" s] "));
}

StatusLogger::StatusLogger(uint32_t statusIntervalMs) {
    statusIntervalMs_ = statusIntervalMs;
    lastStatusMs_ = 0;
    lastState_ = State::Monitoring;  // the controller's starting state
}

void StatusLogger::begin(uint32_t baud) {
    Serial.begin(baud);
    Serial.println(F("Plant waterer started"));
}

void StatusLogger::report(State state, MoistureReading reading, uint16_t raw, uint32_t nowMs) {
    if (state != lastState_) {
        printTimestamp(nowMs);
        Serial.print(stateName(lastState_));
        Serial.print(F(" -> "));
        Serial.println(stateName(state));
        lastState_ = state;
    }

    if (nowMs - lastStatusMs_ >= statusIntervalMs_) {
        printTimestamp(nowMs);
        Serial.print(stateName(state));
        Serial.print(F("  moisture "));
        if (reading.valid) {
            Serial.print(reading.percent);
            Serial.print('%');
        } else {
            Serial.print(F("--"));
        }
        // Raw is printed even when invalid: it's what you read off during calibration.
        Serial.print(F("  raw "));
        Serial.println(raw);
        lastStatusMs_ = nowMs;
    }
}
