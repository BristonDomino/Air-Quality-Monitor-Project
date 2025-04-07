#ifndef SCD41SENSOR_H
#define SCD41SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

class SCD41Sensor {
private:
  SensirionI2cScd4x scd4x;

  // Cache of the last valid reading (filtered)
  bool haveLastReading = false;
  float filteredCO2 = 0.0f;
  float filteredTempC = 0.0f;
  float filteredHumidity = 0.0f;

  // Timing logic
  unsigned long lastReadMs = 0;      
  const unsigned long readInterval = 5000;  // 5 seconds

  // Smoothing factor (0.0 - 1.0). Adjust to taste.
  // Smaller means heavier smoothing (less noise, slower response).
  // Example: alpha=0.2 for moderate smoothing
  const float alpha = 0.2f;

  // Helper to apply exponential smoothing
  float smoothValue(float currentFiltered, float newRaw);

public:
  SCD41Sensor();
  bool begin();

  bool readMeasurement(uint16_t &co2, float &temperatureC, float &humidity);
  bool readMeasurementF(uint16_t &co2, float &temperatureC, float &temperatureF, float &humidity);
};

#endif
