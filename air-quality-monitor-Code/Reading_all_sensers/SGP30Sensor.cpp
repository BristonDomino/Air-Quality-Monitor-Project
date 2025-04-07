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

// Original simple reading method (no humidity compensation)
bool SGP30Sensor::readVOC(uint16_t &voc_ppb) {
  if (!sgp30.IAQmeasure()) {
    Serial.println("SGP30 measurement failed!");
    return false;
  }
  voc_ppb = sgp30.TVOC;  // Total VOC in ppb
  return true;
}

// NEW: Humidity-compensated reading
bool SGP30Sensor::readCalibratedVOC(float temperatureC, float humidityPercent,
                                    uint16_t &voc_ppb, float &voc_mg) {
  // 1) Calculate absolute humidity, scaled for SGP30
  uint16_t absHumidity = computeAbsoluteHumidity(temperatureC, humidityPercent);

  // 2) Pass humidity to SGP30 so it can calibrate internally
  sgp30.setHumidity(absHumidity);

  // 3) Perform measurement
  if (!sgp30.IAQmeasure()) {
    Serial.println("SGP30 measurement failed!");
    return false;
  }
  voc_ppb = sgp30.TVOC;  // TVOC in ppb

  // 4) Convert ppb → mg/m³ (assumes ~100 g/mol for average VOC)
  //    1 ppb = 0.001 ppm, 1 ppm = 1 mg/m³ if MW=24.45 g/mol, but we approximate
  //    A more direct approach is often: mg/m³ = VOC_ppb * 0.00409 for MW=100
  //    (Exact formula depends on the VOC mixture)
  float factor = 0.00409f; // approximate factor for MW=100 g/mol
  voc_mg = voc_ppb * factor;

  return true;
}

// Helper function to compute absolute humidity per Sensirion doc
// returns humidity in mg/m^3, scaled by 1000, so SGP30 can use setHumidity()
uint16_t SGP30Sensor::computeAbsoluteHumidity(float temperatureC, float humidityPercent) {
  // The well-known approximation formula from Sensirion:
  // absolute_humidity (g/m^3) = 216.7 * (rh/100.0 * 6.112 * exp((17.62 * t)/(243.12+t)) / (273.15 + t))
  // Then multiply by 1000 to get mg/m^3, SGP30 expects an integer in 0.01% steps
  float tK = temperatureC + 273.15f;
  float rh = humidityPercent / 100.0f;

  float exp_val = exp((17.62f * temperatureC) / (243.12f + temperatureC));
  float partial_pressure_mbar = 6.112f * exp_val * rh;
  float absHum_gm3 = 216.7f * (partial_pressure_mbar / tK);

  // Convert g/m^3 to mg/m^3
  float absHum_mgm3 = absHum_gm3 * 1000.0f;

  // SGP30 expects an unsigned 16-bit integer in 0.01% steps of humidity
  // So we multiply mg/m^3 by 256, per Sensirion's recommended approach:
  // https://github.com/Sensirion/arduino-sgp/blob/master/src/SensirionI2CSgp30.cpp#L106
  uint16_t absHum_sgp = (uint16_t)(absHum_mgm3 * 256.0f);

  return absHum_sgp;
}
