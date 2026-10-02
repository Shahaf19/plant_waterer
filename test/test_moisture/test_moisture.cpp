// Unit tests for moistureFromRaw(), run on the PC: pio test -e native
#include <unity.h>

#include "moisture.h"

// Fixed test numbers, independent of the real calibration in the config header.
static SensorCalibration testCalibration() {
  SensorCalibration calibration;
  calibration.rawDry = 600;
  calibration.rawWet = 300;
  calibration.marginRaw = 50;
  return calibration;
}

// Unity calls these before/after every test; nothing to prepare here.
void setUp() {}
void tearDown() {}

// 375 is deliberately NOT the midpoint: a reversed formula would give 25 %, and dividing
// before multiplying would give 0 %, so both mistakes fail here.
void test_converts_along_the_dry_to_wet_line() {
  const SensorCalibration calibration = testCalibration();
  MoistureReading reading = moistureFromRaw(375, calibration);
  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_EQUAL_UINT8(75, reading.percent);
  TEST_ASSERT_EQUAL_UINT8(0, moistureFromRaw(600, calibration).percent);
  TEST_ASSERT_EQUAL_UINT8(100, moistureFromRaw(300, calibration).percent);
}

// Exactly at the margin edge: still a real reading, but the raw formula gives -16 % / 116 %
// (what the tutorials' unclamped map() returns), so it must be clamped.
void test_clamps_readings_inside_the_margin() {
  const SensorCalibration calibration = testCalibration();
  MoistureReading drier = moistureFromRaw(650, calibration);
  MoistureReading wetter = moistureFromRaw(250, calibration);
  TEST_ASSERT_TRUE(drier.valid);
  TEST_ASSERT_EQUAL_UINT8(0, drier.percent);
  TEST_ASSERT_TRUE(wetter.valid);
  TEST_ASSERT_EQUAL_UINT8(100, wetter.percent);
}

// One step past the margin on either side means a disconnected or shorted sensor.
void test_rejects_readings_beyond_the_margin() {
  const SensorCalibration calibration = testCalibration();
  TEST_ASSERT_FALSE(moistureFromRaw(651, calibration).valid);
  TEST_ASSERT_FALSE(moistureFromRaw(249, calibration).valid);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_converts_along_the_dry_to_wet_line);
  RUN_TEST(test_clamps_readings_inside_the_margin);
  RUN_TEST(test_rejects_readings_beyond_the_margin);
  return UNITY_END();
}
