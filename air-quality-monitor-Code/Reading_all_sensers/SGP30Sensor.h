#ifndef SGP30SENSOR_H
#define SGP30SENSOR_H

#include <Arduino.h>
#include <Adafruit_SGP30.h>

class SGP30Sensor {
private:
  Adafruit_SGP30 sgp30;

  // Helper to compute absolute humidity scaled for SGP30
  uint16_t computeAbsoluteHumidity(float temperatureC, float humidityPercent);

  // Caching, timing, and smoothing
  bool haveLastReading = false;
  float filteredVOC_ppb = 0.0f;
  float filteredVOC_mg  = 0.0f;

  unsigned long lastReadMs = 0;
  const unsigned long readInterval = 2000; // 2 seconds

  // Smoothing factor. Adjust as needed.
  const float alpha = 0.3f;

  float smoothValue(float currentFiltered, float newRaw);

public:
  SGP30Sensor();
  bool begin();

  // Original readVOC for simple usage (no humidity compensation)
  bool readVOC(uint16_t &voc_ppb);

  // Humidity-compensated data, also returns VOC in mg/m³ if desired
  bool readCalibratedVOC(float temperatureC, float humidityPercent,
                         uint16_t &voc_ppb, float &voc_mg);
};

#endif
