#include "SGP30Sensor.h"
#include <math.h> // for exp()

SGP30Sensor::SGP30Sensor() : sgp30() {}

bool SGP30Sensor::begin() {
  if (!sgp30.begin()) {
    Serial.println("Couldn't find SGP30 sensor! Check connections.");
    return false;
  }
  Serial.println("SGP30 sensor initialized!");

  // Initial baseline (optional, helps accuracy after startup)
  sgp30.IAQinit();

  return true;
}

// Original simple reading method (no humidity compensation, no timing)
bool SGP30Sensor::readVOC(uint16_t &voc_ppb) {
  if (!sgp30.IAQmeasure()) {
    Serial.println("SGP30 measurement failed!");
    return false;
  }
  voc_ppb = sgp30.TVOC;  // Total VOC in ppb
  return true;
}

bool SGP30Sensor::readCalibratedVOC(float temperatureC, float humidityPercent,
                                    uint16_t &voc_ppb, float &voc_mg) {
  unsigned long now = millis();
  bool doFreshRead = (now - lastReadMs >= readInterval);

  if (doFreshRead) {
    // 1) Calculate absolute humidity, scaled for SGP30
    uint16_t absHumidity = computeAbsoluteHumidity(temperatureC, humidityPercent);
    // 2) Pass humidity to SGP30 so it can calibrate internally
    sgp30.setHumidity(absHumidity);

    // 3) Perform measurement
    if (!sgp30.IAQmeasure()) {
      Serial.println("SGP30 measurement failed!");
      if (!haveLastReading) return false; // no fallback
      // fallback
      voc_ppb = lastVOC_ppb;
      voc_mg = lastVOC_mg;
      return true;
    }
    uint16_t newVOC_ppb = sgp30.TVOC;

    // 4) Convert ppb → mg/m³
    float factor = 0.00409f; // approximate factor for MW=100 g/mol
    float newVOC_mg = newVOC_ppb * factor;

    // Update cache
    lastVOC_ppb = newVOC_ppb;
    lastVOC_mg  = newVOC_mg;
    haveLastReading = true;
    lastReadMs = now;
  }

  // Return either newly read or cached data
  if (!haveLastReading) {
    // No reading to return yet
    return false;
  }
  voc_ppb = lastVOC_ppb;
  voc_mg  = lastVOC_mg;

  return true;
}

// Helper function to compute absolute humidity per Sensirion doc
// returns humidity in mg/m^3, scaled by 1000, so SGP30 can use setHumidity()
uint16_t SGP30Sensor::computeAbsoluteHumidity(float temperatureC, float humidityPercent) {
  float tK = temperatureC + 273.15f;
  float rh = humidityPercent / 100.0f;

  float exp_val = exp((17.62f * temperatureC) / (243.12f + temperatureC));
  float partial_pressure_mbar = 6.112f * exp_val * rh;
  float absHum_gm3 = 216.7f * (partial_pressure_mbar / tK);

  // Convert g/m^3 to mg/m^3
  float absHum_mgm3 = absHum_gm3 * 1000.0f;

  // SGP30 expects an unsigned 16-bit integer in 0.01% steps of humidity
  uint16_t absHum_sgp = (uint16_t)(absHum_mgm3 * 256.0f);

  return absHum_sgp;
}
