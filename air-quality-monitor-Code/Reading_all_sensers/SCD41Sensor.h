#ifndef SCD41SENSOR_H
#define SCD41SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

class SCD41Sensor {
private:
  SensirionI2cScd4x scd4x;

  // Cache of the last valid reading
  bool haveLastReading = false;
  uint16_t lastCO2 = 0;
  float lastTempC = 0.0f;
  float lastHumidity = 0.0f;

  // New: timing logic
  unsigned long lastReadMs = 0;      // the last time we did a fresh read
  const unsigned long readInterval = 5000;  // 5 seconds

public:
  SCD41Sensor();
  bool begin();

  // Always returns a reading (fresh or cached) if available
  bool readMeasurement(uint16_t &co2, float &temperatureC, float &humidity);
  bool readMeasurementF(uint16_t &co2, float &temperatureC, float &temperatureF, float &humidity);
};

#endif
