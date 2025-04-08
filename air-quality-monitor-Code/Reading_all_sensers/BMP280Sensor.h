#ifndef BMP280SENSOR_H
#define BMP280SENSOR_H

#include <Arduino.h>
#include <Adafruit_BMP280.h>

class BMP280Sensor {
private:
  Adafruit_BMP280 bmp;
  bool isConnected = false; // NEW

  float seaLevelPressure_hPa;

  bool haveLastReading = false;
  float filteredPressure_inhg = 0.0f;
  float filteredAlt_m        = 0.0f;
  float filteredAlt_ft       = 0.0f;

  const float alpha = 0.2f; 
  float smoothValue(float currentFiltered, float newRaw);

public:
  // NEW: maxRetries
  BMP280Sensor(float seaLevel_hPa = 1013.25);
  bool begin(uint8_t addr = 0x77, int maxRetries = 1);

  bool connected() const { return isConnected; }

  bool readData(float &pressure_inhg, float &alt_meters, float &alt_feet);
};

#endif
