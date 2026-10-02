#pragma once

#include <stdint.h>

// Raw sensor values measured in real soil. A capacitive sensor reads LOWER when wetter.
struct SensorCalibration {
    uint16_t rawDry;
    uint16_t rawWet;
    uint16_t marginRaw;  // how far past dry/wet a reading may go before it counts as a fault
};

struct MoistureReading {
    bool valid;       // false = sensor disconnected or shorted
    uint8_t percent;  // 0 = dry, 100 = wet; only meaningful when valid
};

MoistureReading moistureFromRaw(uint16_t raw, SensorCalibration calibration);
