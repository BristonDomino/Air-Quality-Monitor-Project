#include "BMP280Sensor.h"

// 1 hPa = 100 Pa
// 1 inHg = 3386.39 Pa
static const float PA_PER_INHG = 3386.39;
static const float FEET_PER_METER = 3.28084;

BMP280Sensor::BMP280Sensor(float seaLevel_hPa) :
  seaLevelPressure_hPa(seaLevel_hPa) { }

bool BMP280Sensor::begin(uint8_t addr) {
  // Initialize the sensor with the default I2C address (0x77) on the default Wire bus.
  if (!bmp.begin(addr)) {
    Serial.println("Could not find BMP280 sensor! Check wiring or address.");
    return false;
  }
  // Optionally configure more advanced settings here...
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Normal mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temp oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time
  
  Serial.println("BMP280 sensor initialized!");
  return true;
}

bool BMP280Sensor::readData(float &pressure_inhg, float &alt_meters, float &alt_feet) {
  // Read raw temperature in Celsius (not displayed but needed for pressure calc).
  // float temperatureC = bmp.readTemperature(); // Optional if you want temperature
  
  // Pressure is returned in Pascals by default
  float pressure_pa = bmp.readPressure();
  if (pressure_pa == 0) {
    // Possibly indicates a read failure
    return false;
  }

  // Convert Pa → inHg
  pressure_inhg = pressure_pa / PA_PER_INHG;

  // Use the library's built-in altitude function (returns altitude in meters).
  // By default, you pass your local sea-level pressure in hPa.
  alt_meters = bmp.readAltitude(seaLevelPressure_hPa);
  alt_feet   = alt_meters * FEET_PER_METER;

  return true;
}
