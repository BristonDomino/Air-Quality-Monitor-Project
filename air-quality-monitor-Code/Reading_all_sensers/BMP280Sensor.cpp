#include "BMP280Sensor.h"

static const float PA_PER_INHG = 3386.39;
static const float FEET_PER_METER = 3.28084;

BMP280Sensor::BMP280Sensor(float seaLevel_hPa)
  : seaLevelPressure_hPa(seaLevel_hPa) {}

bool BMP280Sensor::begin(uint8_t addr, int maxRetries) {
  isConnected = false;

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    Serial.print("BMP280: Attempting init (Attempt ");
    Serial.print(attempt);
    Serial.print("/");
    Serial.print(maxRetries);
    Serial.println(")...");

    if (!bmp.begin(addr)) {
      Serial.println("Could not find BMP280 sensor! Check wiring or address...");
      delay(200);
      continue;
    }

    // bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
    //                 Adafruit_BMP280::SAMPLING_X2,
    //                 Adafruit_BMP280::SAMPLING_X16,
    //                 Adafruit_BMP280::FILTER_X2,
    //                 Adafruit_BMP280::STANDBY_MS_500);
    //
    Serial.println("BMP280 sensor initialized!");
    isConnected = true;
    break;
  }

  if (!isConnected) {
    Serial.println("BMP280: Failed to initialize after retries.");
  }
  return isConnected;
}

float BMP280Sensor::smoothValue(float currentFiltered, float newRaw) {
  return currentFiltered + alpha * (newRaw - currentFiltered);
}

bool BMP280Sensor::readData(float &pressure_inhg_out, float &alt_m_out, float &alt_ft_out) {
  if (!isConnected) {
    return false;
  }

  float pressure_pa = bmp.readPressure();
  if (pressure_pa <= 0.0f) {
    return false;
  }

  float rawPressure_inhg = pressure_pa / PA_PER_INHG;
  float rawAlt_m  = bmp.readAltitude(seaLevelPressure_hPa);
  float rawAlt_ft = rawAlt_m * FEET_PER_METER;

  if (!haveLastReading) {
    // First reading
    filteredPressure_inhg = rawPressure_inhg;
    filteredAlt_m         = rawAlt_m;
    filteredAlt_ft        = rawAlt_ft;
    haveLastReading       = true;
  } else {
    filteredPressure_inhg = smoothValue(filteredPressure_inhg, rawPressure_inhg);
    filteredAlt_m         = smoothValue(filteredAlt_m, rawAlt_m);
    filteredAlt_ft        = smoothValue(filteredAlt_ft, rawAlt_ft);
  }

  pressure_inhg_out = filteredPressure_inhg;
  alt_m_out         = filteredAlt_m;
  alt_ft_out        = filteredAlt_ft;

  return true;
}
