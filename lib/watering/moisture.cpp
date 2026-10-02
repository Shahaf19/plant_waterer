#include "moisture.h"

MoistureReading moistureFromRaw(uint16_t raw, SensorCalibration calibration) {
    // int32_t, not uint16_t: the math below can go negative, and past 32767 (an int on the Uno).
    const int32_t dry = calibration.rawDry;
    const int32_t wet = calibration.rawWet;
    const int32_t validHigh = dry + calibration.marginRaw;
    const int32_t validLow = wet - calibration.marginRaw;

    const bool valid = raw >= validLow && raw <= validHigh;
    if (!valid) {
        return {false, 0};
    }

    int32_t percent = (dry - raw) * 100 / (dry - wet);

    // Inside the margin the formula lands slightly below 0 or above 100.
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    return {true, static_cast<uint8_t>(percent)};
}
