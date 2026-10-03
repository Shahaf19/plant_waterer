#include "pump.h"

#include <Arduino.h>

Pump::Pump(uint8_t pin, bool activeLow) {
    pin_ = pin;
    activeLow_ = activeLow;
}

void Pump::begin() {
    // OFF level first, then output: in the other order the pin briefly drives LOW,
    // which switches an active-LOW relay ON at every boot.
    set(false);
    pinMode(pin_, OUTPUT);
}

void Pump::set(bool on) {
    // Active-LOW relay: on = LOW. Active-HIGH relay: on = HIGH.
    if ((on && activeLow_) || (!on && !activeLow_)) {
        digitalWrite(pin_, LOW);
    } else {
        digitalWrite(pin_, HIGH);
    }
}
