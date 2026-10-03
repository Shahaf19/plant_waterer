#include "soil_sensor.h"

#include <Arduino.h>

SoilSensor::SoilSensor(uint8_t pin) {
    pin_ = pin;
}

uint16_t SoilSensor::readRaw() const {
    // analogRead returns an int, but the Uno's ADC only produces 0-1023.
    return static_cast<uint16_t>(analogRead(pin_));
}
