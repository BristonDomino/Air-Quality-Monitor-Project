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

// Simple reading, no smoothing logic here (kept as-is)
bool SGP30Sensor::readVOC(uint16_t &voc_ppb) {
  if (!sgp30.IAQmeasure()) {
    Serial.println("SGP30 measurement failed!");
    return false;
  }
  voc_ppb = sgp30.TVOC;  // Total VOC in ppb
  return true;
}

// Exponential smoothing helper
float SGP30Sensor::smoothValue(float currentFiltered, float newRaw) {
  return currentFiltered + alpha * (newRaw - currentFiltered);
}

bool SGP30Sensor::readCalibratedVOC(float temperatureC, float humidityPercent,
                                    uint16_t &voc_ppb_out, float &voc_mg_out) {
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
      if (!haveLastReading) return false;
      // fallback
      voc_ppb_out = (uint16_t)filteredVOC_ppb;
      voc_mg_out  = filteredVOC_mg;
      return true;
    }
    uint16_t rawVOC_ppb = sgp30.TVOC;

    // 4) Convert ppb → mg/m³
    float factor = 0.00409f; // approximate factor for MW=100 g/mol
    float rawVOC_mg = rawVOC_ppb * factor;

    // Now apply smoothing if we have an existing baseline
    if (!haveLastReading) {
      // First reading
      filteredVOC_ppb = (float)rawVOC_ppb;
      filteredVOC_mg  = rawVOC_mg;
      haveLastReading = true;
    } else {
      filteredVOC_ppb = smoothValue(filteredVOC_ppb, (float)rawVOC_ppb);
      filteredVOC_mg  = smoothValue(filteredVOC_mg, rawVOC_mg);
    }

    lastReadMs = now;
  }

  // Return cached (smoothed) data
  if (!haveLastReading) {
    return false;
  }
  voc_ppb_out = (uint16_t)filteredVOC_ppb;
  voc_mg_out  = filteredVOC_mg;
  return true;
}

// Helper function to compute absolute humidity
uint16_t SGP30Sensor::computeAbsoluteHumidity(float temperatureC, float humidityPercent) {
  float tK = temperatureC + 273.15f;
  float rh = humidityPercent / 100.0f;

  float exp_val = exp((17.62f * temperatureC) / (243.12f + temperatureC));
  float partial_pressure_mbar = 6.112f * exp_val * rh;
  float absHum_gm3 = 216.7f * (partial_pressure_mbar / tK);

  float absHum_mgm3 = absHum_gm3 * 1000.0f;

  uint16_t absHum_sgp = (uint16_t)(absHum_mgm3 * 256.0f);

  return absHum_sgp;
}
