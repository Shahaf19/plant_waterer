#pragma once

// Every tunable number of the system, in one place. Only main.cpp includes this file;
// the logic modules get their numbers passed in, so tests can use their own.

#include <Arduino.h>

#include "watering_controller.h"

// --- Wiring (decision #12) ---
constexpr uint8_t PIN_SENSOR = A0;
constexpr uint8_t PIN_RELAY = 10;     // not 13: the bootloader blinks pin 13 at every reset
constexpr bool RELAY_ACTIVE_LOW = true;  // module says "low level trigger"

// --- Sensor calibration (placeholders until Milestone 1; read "raw" off the status line) ---
constexpr uint16_t SENSOR_RAW_DRY = 600;    // sensor in dry soil
constexpr uint16_t SENSOR_RAW_WET = 300;    // sensor in freshly watered soil
constexpr uint16_t SENSOR_MARGIN_RAW = 50;  // further out than this = disconnected/shorted sensor

static_assert(SENSOR_RAW_DRY > SENSOR_RAW_WET,
              "A capacitive sensor reads lower when wetter: dry must be above wet");

// --- Plant presets (decision #15). Rough starting points; tune by watching the plant. ---
// Ordered from the plant that likes it driest to the one that likes it wettest.
constexpr WateringSettings CACTUS = {
    15,      // startBelowPercent
    30,      // stopAtPercent
    3000,    // doseMs
    900000,  // soakMs (15 min: dry, sandy soil drains slowly to the sensor)
    3,       // maxDosesInRow
};

constexpr WateringSettings HERBS = {
    35,      // startBelowPercent
    55,      // stopAtPercent
    5000,    // doseMs
    600000,  // soakMs (10 min)
    3,       // maxDosesInRow
};

constexpr WateringSettings LEMONGRASS = {
    40,      // startBelowPercent: thirsty, but needs well-drained soil
    60,      // stopAtPercent
    5000,    // doseMs
    600000,  // soakMs (10 min)
    3,       // maxDosesInRow
};

constexpr WateringSettings MINT = {
    45,      // startBelowPercent: wilts quickly when the soil dries out
    65,      // stopAtPercent
    5000,    // doseMs
    600000,  // soakMs (10 min)
    3,       // maxDosesInRow
};

constexpr WateringSettings FERN = {
    50,      // startBelowPercent
    70,      // stopAtPercent
    5000,    // doseMs
    600000,  // soakMs (10 min)
    3,       // maxDosesInRow
};

constexpr WateringSettings ACTIVE_PLANT = HERBS;  // <-- pick the plant here

static_assert(ACTIVE_PLANT.startBelowPercent < ACTIVE_PLANT.stopAtPercent,
              "Hysteresis needs a gap: start-below must be lower than stop-at");

// --- Status output over USB (decision #13) ---
constexpr uint32_t SERIAL_BAUD = 115200;  // must match monitor_speed in platformio.ini
constexpr uint32_t STATUS_INTERVAL_MS = 5000;
