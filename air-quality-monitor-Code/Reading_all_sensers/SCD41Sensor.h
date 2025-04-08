#ifndef SCD41SENSOR_H
#define SCD41SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

class SCD41Sensor {
private:
  SensirionI2cScd4x scd4x;
  bool isConnected = false;

  // Error tracking
  int consecutiveErrorCount = 0;    // how many times in a row we've had an I2C error
  const int maxConsecutiveErrors = 3; // after 3 consecutive errors, try bus recovery

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

  // Attempt to recover the I2C bus and re-init sensor
  void recoverI2CBus();

public:
  SCD41Sensor();
  bool begin(int maxRetries = 1);
  bool connected() const { return isConnected; }

  bool readMeasurement(uint16_t &co2, float &temperatureC, float &humidity);
  bool readMeasurementF(uint16_t &co2, float &temperatureC, float &temperatureF, float &humidity);
};

#endif
