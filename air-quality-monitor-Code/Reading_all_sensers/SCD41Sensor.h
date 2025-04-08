#ifndef SCD41SENSOR_H
#define SCD41SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

class SCD41Sensor {
private:
  SensirionI2cScd4x scd4x;
  bool isConnected = false; // NEW: tracks whether sensor initialized successfully

  // Cache of the last valid reading (filtered)
  bool haveLastReading = false;
  float filteredCO2 = 0.0f;
  float filteredTempC = 0.0f;
  float filteredHumidity = 0.0f;

  // Timing logic
  unsigned long lastReadMs = 0;
  const unsigned long readInterval = 5000; // 5 seconds

  // Smoothing factor
  const float alpha = 0.2f;
  float smoothValue(float currentFiltered, float newRaw);

public:
  SCD41Sensor();
  
  // NEW: maxRetries parameter if you want repeated attempts
  bool begin(int maxRetries = 1);

  bool connected() const { return isConnected; }

  bool readMeasurement(uint16_t &co2, float &temperatureC, float &humidity);
  bool readMeasurementF(uint16_t &co2, float &temperatureC, float &temperatureF, float &humidity);
};

#endif
