#ifndef BMP280SENSOR_H
#define BMP280SENSOR_H

#include <Arduino.h>
#include <Adafruit_BMP280.h>

class BMP280Sensor {
private:
  Adafruit_BMP280 bmp;         // BMP280 object
  float seaLevelPressure_hPa;  // Reference sea-level pressure in hPa

public:
  BMP280Sensor(float seaLevel_hPa = 1013.25); 
  bool begin(uint8_t addr = 0x77);   
  bool readData(float &pressure_inhg, float &alt_meters, float &alt_feet);
};

#endif
