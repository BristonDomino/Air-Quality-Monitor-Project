#ifndef BMP280SENSOR_H
#define BMP280SENSOR_H

#include <Arduino.h>
#include <Adafruit_BMP280.h>

class BMP280Sensor {
private:
  Adafruit_BMP280 bmp;         
  float seaLevelPressure_hPa;  

  // Smoothing
  bool haveLastReading = false;
  float filteredPressure_inhg = 0.0f;
  float filteredAlt_m        = 0.0f;
  float filteredAlt_ft       = 0.0f;

  // Smoothing factor for barometric data
  const float alpha = 0.2f; 

  // Helper to apply exponential smoothing
  float smoothValue(float currentFiltered, float newRaw);

public:
  BMP280Sensor(float seaLevel_hPa = 1013.25); 
  bool begin(uint8_t addr = 0x77);   

  // The readData function now returns the *smoothed* values
  bool readData(float &pressure_inhg, float &alt_meters, float &alt_feet);
};

#endif
