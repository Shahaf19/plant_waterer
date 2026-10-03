#pragma once

#include <stdint.h>

// Drives the pump's relay. Hides the relay's polarity: callers only ever say on or off.
class Pump {
    public:
    Pump(uint8_t pin, bool activeLow);

    // Call once from setup(). Not in the constructor: global objects are built
    // before the Arduino framework has prepared the chip.
    void begin();
    void set(bool on);

    private:
    uint8_t pin_;
    bool activeLow_;
};
