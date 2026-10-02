// Unit tests for WateringController, run on the PC: pio test -e native
// Each test is a timeline: feed readings at chosen times, then check the state.
#include <unity.h>

#include "watering_controller.h"

using State = WateringController::State;

// Fixed test numbers, independent of the real settings in the config header.
static WateringSettings testSettings() {
    WateringSettings settings;
    settings.startBelowPercent = 40;
    settings.stopAtPercent = 60;
    settings.doseMs = 5000;
    settings.soakMs = 600000;
    settings.maxDosesInRow = 3;
    return settings;
}

static MoistureReading moisture(uint8_t percent) {
    return {true, percent};
}

static const MoistureReading kBrokenSensor = {false, 0};

void setUp() {}
void tearDown() {}

void test_starts_in_monitoring_with_pump_off() {
    WateringController controller(testSettings());
    TEST_ASSERT_TRUE(controller.state() == State::Monitoring);
    TEST_ASSERT_FALSE(controller.isPumpOn());
}

// Hysteresis, lower side: 50 % is between the thresholds, so it must NOT start watering.
void test_starts_dosing_only_below_the_lower_threshold() {
    WateringController controller(testSettings());
    controller.update(moisture(50), 0);
    TEST_ASSERT_TRUE(controller.state() == State::Monitoring);

    controller.update(moisture(39), 1000);
    TEST_ASSERT_TRUE(controller.state() == State::Dosing);
    TEST_ASSERT_TRUE(controller.isPumpOn());
}

// Safety: the dose ends on time even though the sensor still says dry.
void test_dose_stops_on_time_even_if_soil_stays_dry() {
    WateringController controller(testSettings());
    controller.update(moisture(10), 0);
    controller.update(moisture(10), 4999);
    TEST_ASSERT_TRUE(controller.isPumpOn());

    controller.update(moisture(10), 5000);
    TEST_ASSERT_TRUE(controller.state() == State::Soaking);
    TEST_ASSERT_FALSE(controller.isPumpOn());
}

// Soak: a dry reading during the soak must not trigger another dose; after it, wet ends the cycle.
void test_soaks_before_trusting_the_sensor_then_returns_to_monitoring() {
    WateringController controller(testSettings());
    controller.update(moisture(10), 0);
    controller.update(moisture(10), 5000);

    controller.update(moisture(10), 5000 + 599999);
    TEST_ASSERT_TRUE(controller.state() == State::Soaking);

    controller.update(moisture(60), 5000 + 600000);
    TEST_ASSERT_TRUE(controller.state() == State::Monitoring);
}

// Hysteresis, upper side: 50 % would not START watering, but it doesn't FINISH a cycle either.
void test_doses_again_after_soak_if_below_the_upper_threshold() {
    WateringController controller(testSettings());
    controller.update(moisture(10), 0);
    controller.update(moisture(10), 5000);

    controller.update(moisture(50), 5000 + 600000);
    TEST_ASSERT_TRUE(controller.state() == State::Dosing);
}

// Empty tank / stuck sensor: every allowed dose given, soil still dry, so stop for good.
void test_faults_when_max_doses_do_not_help() {
    const WateringSettings settings = testSettings();
    WateringController controller(settings);
    uint32_t nowMs = 0;
    controller.update(moisture(10), nowMs);

    for (int dose = 0; dose < settings.maxDosesInRow; dose++) {
        nowMs += settings.doseMs;
        controller.update(moisture(10), nowMs);  // dose ends
        nowMs += settings.soakMs;
        controller.update(moisture(10), nowMs);  // soak ends: dose again, or fault after the last
    }

    TEST_ASSERT_TRUE(controller.state() == State::Fault);
    TEST_ASSERT_FALSE(controller.isPumpOn());
}

// The most dangerous moment for a broken sensor is mid-dose: pump must stop and stay stopped.
void test_broken_sensor_stops_the_pump_and_fault_is_latched() {
    WateringController controller(testSettings());
    controller.update(moisture(10), 0);
    controller.update(kBrokenSensor, 1000);
    TEST_ASSERT_TRUE(controller.state() == State::Fault);
    TEST_ASSERT_FALSE(controller.isPumpOn());

    controller.update(moisture(10), 10000000);
    TEST_ASSERT_TRUE(controller.state() == State::Fault);
}

// millis() wraps to 0 after ~49.7 days. A naive "now >= start + dose" check would end this
// dose at the first update after the start, because start + dose itself wraps to a small number.
void test_dose_timing_survives_clock_wraparound() {
    WateringController controller(testSettings());
    const uint32_t startMs = UINT32_MAX - 1000;
    controller.update(moisture(10), startMs);

    controller.update(moisture(10), startMs + 1000);  // = UINT32_MAX, just before the wrap
    TEST_ASSERT_TRUE(controller.isPumpOn());

    controller.update(moisture(10), startMs + 5000);  // wrapped past 0
    TEST_ASSERT_FALSE(controller.isPumpOn());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_starts_in_monitoring_with_pump_off);
    RUN_TEST(test_starts_dosing_only_below_the_lower_threshold);
    RUN_TEST(test_dose_stops_on_time_even_if_soil_stays_dry);
    RUN_TEST(test_soaks_before_trusting_the_sensor_then_returns_to_monitoring);
    RUN_TEST(test_doses_again_after_soak_if_below_the_upper_threshold);
    RUN_TEST(test_faults_when_max_doses_do_not_help);
    RUN_TEST(test_broken_sensor_stops_the_pump_and_fault_is_latched);
    RUN_TEST(test_dose_timing_survives_clock_wraparound);
    return UNITY_END();
}
