#pragma once

#include <stdint.h>

// Reads the capacitive soil moisture sensor's raw value (0-1023) from an analog pin.
class SoilSensor {
    public:
    SoilSensor(uint8_t pin);
    uint16_t readRaw() const;

    private:
    uint8_t pin_;
};
