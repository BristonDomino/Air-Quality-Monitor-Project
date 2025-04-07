#ifndef SGP30SENSOR_H
#define SGP30SENSOR_H

#include <Arduino.h>
#include <Adafruit_SGP30.h>

class SGP30Sensor {
private:
  Adafruit_SGP30 sgp30;

  // Helper to compute absolute humidity scaled for SGP30
  uint16_t computeAbsoluteHumidity(float temperatureC, float humidityPercent);

public:
  SGP30Sensor();
  bool begin();

  // Original readVOC for simple usage
  bool readVOC(uint16_t &voc_ppb);

  // NEW: readCalibratedVOC for humidity-compensated data
  // Also returns VOC in mg/m³ if desired
  bool readCalibratedVOC(float temperatureC, float humidityPercent,
                         uint16_t &voc_ppb, float &voc_mg);
};

#endif
