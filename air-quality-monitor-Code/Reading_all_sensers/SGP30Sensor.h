#ifndef SGP30SENSOR_H
#define SGP30SENSOR_H

#include <Arduino.h>
#include <Adafruit_SGP30.h>

class SGP30Sensor {
private:
  Adafruit_SGP30 sgp30;
  bool isConnected = false; // NEW

  // Helper to compute absolute humidity
  uint16_t computeAbsoluteHumidity(float temperatureC, float humidityPercent);

  // Caching, timing, and smoothing
  bool haveLastReading = false;
  float filteredVOC_ppb = 0.0f;
  float filteredVOC_mg  = 0.0f;

  unsigned long lastReadMs = 0;
  const unsigned long readInterval = 2000; // 2 seconds
  const float alpha = 0.3f;
  float smoothValue(float currentFiltered, float newRaw);

public:
  SGP30Sensor();

  // NEW: maxRetries
  bool begin(int maxRetries = 1);
  
  bool connected() const { return isConnected; }

  bool readVOC(uint16_t &voc_ppb);
  bool readCalibratedVOC(float temperatureC, float humidityPercent,
                         uint16_t &voc_ppb, float &voc_mg);
};

#endif
