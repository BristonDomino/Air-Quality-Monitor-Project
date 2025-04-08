#include "SGP30Sensor.h"
#include <math.h> // for exp()

SGP30Sensor::SGP30Sensor() : sgp30() {}

bool SGP30Sensor::begin(int maxRetries) {
  isConnected = false;

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    Serial.print("SGP30: Attempting init (Attempt ");
    Serial.print(attempt);
    Serial.print("/");
    Serial.print(maxRetries);
    Serial.println(")...");

    if (sgp30.begin()) {
      Serial.println("SGP30 sensor initialized!");
      sgp30.IAQinit(); // optional baseline initialization
      isConnected = true;
      break;
    } else {
      Serial.println("Couldn't find SGP30 sensor on this attempt...");
      delay(200); // small delay before retry
    }
  }

  if (!isConnected) {
    Serial.println("SGP30: Failed to initialize after retries.");
  }
  return isConnected;
}

bool SGP30Sensor::readVOC(uint16_t &voc_ppb) {
  // If not connected, skip
  if (!isConnected) {
    return false;
  }
  // Simple read, no smoothing/time
  if (!sgp30.IAQmeasure()) {
    Serial.println("SGP30 measurement failed!");
    return false;
  }
  voc_ppb = sgp30.TVOC;
  return true;
}

float SGP30Sensor::smoothValue(float currentFiltered, float newRaw) {
  return currentFiltered + alpha * (newRaw - currentFiltered);
}

bool SGP30Sensor::readCalibratedVOC(float temperatureC, float humidityPercent,
                                    uint16_t &voc_ppb_out, float &voc_mg_out) {
  if (!isConnected) {
    return false;
  }

  unsigned long now = millis();
  bool doFreshRead = (now - lastReadMs >= readInterval);

  if (doFreshRead) {
    // humidity compensation
    uint16_t absHumidity = computeAbsoluteHumidity(temperatureC, humidityPercent);
    sgp30.setHumidity(absHumidity);

    if (!sgp30.IAQmeasure()) {
      Serial.println("SGP30 measurement failed!");
      if (!haveLastReading) return false;
      voc_ppb_out = (uint16_t)filteredVOC_ppb;
      voc_mg_out  = filteredVOC_mg;
      return true;
    }
    uint16_t rawVOC_ppb = sgp30.TVOC;
    float factor = 0.00409f;
    float rawVOC_mg = rawVOC_ppb * factor;

    if (!haveLastReading) {
      // first reading
      filteredVOC_ppb = (float)rawVOC_ppb;
      filteredVOC_mg  = rawVOC_mg;
      haveLastReading = true;
    } else {
      filteredVOC_ppb = smoothValue(filteredVOC_ppb, (float)rawVOC_ppb);
      filteredVOC_mg  = smoothValue(filteredVOC_mg,  rawVOC_mg);
    }
    lastReadMs = now;
  }

  if (!haveLastReading) {
    return false;
  }
  voc_ppb_out = (uint16_t)filteredVOC_ppb;
  voc_mg_out  = filteredVOC_mg;
  return true;
}

uint16_t SGP30Sensor::computeAbsoluteHumidity(float temperatureC, float humidityPercent) {
  float tK = temperatureC + 273.15f;
  float rh = humidityPercent / 100.0f;

  float exp_val = exp((17.62f * temperatureC) / (243.12f + temperatureC));
  float partial_pressure_mbar = 6.112f * exp_val * rh;
  float absHum_gm3 = 216.7f * (partial_pressure_mbar / tK);

  float absHum_mgm3 = absHum_gm3 * 1000.0f;
  return (uint16_t)(absHum_mgm3 * 256.0f);
}
