#include <Arduino.h>

#include "config.h"
#include "moisture.h"
#include "pump.h"
#include "soil_sensor.h"
#include "status_logger.h"
#include "watering_controller.h"

// File-level, so they live for the whole program: loop() must see the same controller
// (and its memory) on every pass. static = only this file can touch them.
static SoilSensor sensor(PIN_SENSOR);
static Pump pump(PIN_RELAY, RELAY_ACTIVE_LOW);
static WateringController controller(ACTIVE_PLANT);
static StatusLogger logger(STATUS_INTERVAL_MS);

static const SensorCalibration calibration = {SENSOR_RAW_DRY, SENSOR_RAW_WET, SENSOR_MARGIN_RAW};

void setup() {
    pump.begin();  // first: relay safely OFF before anything else happens
    logger.begin(SERIAL_BAUD);
}

void loop() {
    const uint32_t nowMs = millis();  // read once, so every step sees the same time
    const uint16_t raw = sensor.readRaw();
    const MoistureReading reading = moistureFromRaw(raw, calibration);

    controller.update(reading, nowMs);
    pump.set(controller.isPumpOn());
    logger.report(controller.state(), reading, raw, nowMs);
}
