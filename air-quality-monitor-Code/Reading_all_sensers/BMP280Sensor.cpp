#include "BMP280Sensor.h"

// 1 hPa = 100 Pa
// 1 inHg = 3386.39 Pa
static const float PA_PER_INHG = 3386.39;
static const float FEET_PER_METER = 3.28084;

BMP280Sensor::BMP280Sensor(float seaLevel_hPa)
  : seaLevelPressure_hPa(seaLevel_hPa) {}

bool BMP280Sensor::begin(uint8_t addr) {
  // Initialize the sensor with the default I2C address (0x77)
  if (!bmp.begin(addr)) {
    Serial.println("Could not find BMP280 sensor! Check wiring or address.");
    return false;
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,  
                  Adafruit_BMP280::SAMPLING_X16, 
                  Adafruit_BMP280::FILTER_X16,   
                  Adafruit_BMP280::STANDBY_MS_500);

  Serial.println("BMP280 sensor initialized!");
  return true;
}

// Helper for exponential smoothing
float BMP280Sensor::smoothValue(float currentFiltered, float newRaw) {
  return currentFiltered + alpha * (newRaw - currentFiltered);
}

bool BMP280Sensor::readData(float &pressure_inhg_out, float &alt_m_out, float &alt_ft_out) {
  // For demonstration, we won't do a time-based approach here
  // but you could add it similarly to the other sensors if needed.

  float pressure_pa = bmp.readPressure();
  if (pressure_pa <= 0.0f) {
    return false;
  }

  // Convert Pa → inHg
  float rawPressure_inhg = pressure_pa / PA_PER_INHG;

  // Altitude in meters
  float rawAlt_m = bmp.readAltitude(seaLevelPressure_hPa);
  // And feet
  float rawAlt_ft = rawAlt_m * FEET_PER_METER;

  if (!haveLastReading) {
    // First reading, initialize filters
    filteredPressure_inhg = rawPressure_inhg;
    filteredAlt_m         = rawAlt_m;
    filteredAlt_ft        = rawAlt_ft;
    haveLastReading       = true;
  } else {
    // Apply smoothing
    filteredPressure_inhg = smoothValue(filteredPressure_inhg, rawPressure_inhg);
    filteredAlt_m         = smoothValue(filteredAlt_m, rawAlt_m);
    filteredAlt_ft        = smoothValue(filteredAlt_ft, rawAlt_ft);
  }

  // Return the smoothed data
  pressure_inhg_out = filteredPressure_inhg;
  alt_m_out         = filteredAlt_m;
  alt_ft_out        = filteredAlt_ft;

  return true;
}
