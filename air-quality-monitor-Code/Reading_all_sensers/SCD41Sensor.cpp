#include "SCD41Sensor.h"

SCD41Sensor::SCD41Sensor() : scd4x() {}

bool SCD41Sensor::begin(int maxRetries) {
  Wire.begin();
  isConnected = false; // start off false

  uint16_t error;
  char errorMessage[256];

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    Serial.print("SCD41: Attempting to initialize (Attempt ");
    Serial.print(attempt);
    Serial.print("/");
    Serial.print(maxRetries);
    Serial.println(")...");

    scd4x.begin(Wire, 0x62); // Default I2C address

    // Stop previously started measurement
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
      Serial.print("Error stopping periodic measurement: ");
      errorToString(error, errorMessage, sizeof(errorMessage));
      Serial.println(errorMessage);
      delay(200); // short delay before retry
      continue;   // try again
    }

    delay(500);

    // Start periodic measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
      Serial.print("Error starting periodic measurement: ");
      errorToString(error, errorMessage, sizeof(errorMessage));
      Serial.println(errorMessage);
      delay(200);
      continue;
    }

    // If we reach here, initialization succeeded
    isConnected = true;
    Serial.println("SCD-41 sensor initialized successfully!");
    break;
  }

  if (!isConnected) {
    Serial.println("SCD41: Failed to initialize after retries.");
  }
  return isConnected;
}

float SCD41Sensor::smoothValue(float currentFiltered, float newRaw) {
  return currentFiltered + alpha * (newRaw - currentFiltered);
}

bool SCD41Sensor::readMeasurement(uint16_t &co2_out, float &tempC_out, float &hum_out) {
  // If the sensor never connected, no point in reading
  if (!isConnected) {
    return false;
  }

  unsigned long now = millis();
  bool doFreshRead = (now - lastReadMs >= readInterval);

  if (doFreshRead) {
    uint16_t error;
    char errorMessage[256];
    bool dataReady = false;

    error = scd4x.getDataReadyStatus(dataReady);
    if (error) {
      Serial.print("SCD41: Error reading data ready status: ");
      errorToString(error, errorMessage, sizeof(errorMessage));
      Serial.println(errorMessage);
      if (!haveLastReading) return false;
      co2_out   = (uint16_t)filteredCO2;
      tempC_out = filteredTempC;
      hum_out   = filteredHumidity;
      return true;
    }

    if (dataReady) {
      uint16_t rawCO2;
      float rawTempC, rawHum;

      error = scd4x.readMeasurement(rawCO2, rawTempC, rawHum);
      if (error) {
        Serial.print("SCD41: Error reading measurement: ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
        if (!haveLastReading) return false;
        co2_out   = (uint16_t)filteredCO2;
        tempC_out = filteredTempC;
        hum_out   = filteredHumidity;
        return true;
      }
      // Fresh data
      if (!haveLastReading) {
        // First reading
        filteredCO2      = (float)rawCO2;
        filteredTempC    = rawTempC;
        filteredHumidity = rawHum;
        haveLastReading  = true;
      } else {
        // Exponential smoothing
        filteredCO2      = smoothValue(filteredCO2, (float)rawCO2);
        filteredTempC    = smoothValue(filteredTempC, rawTempC);
        filteredHumidity = smoothValue(filteredHumidity, rawHum);
      }
      lastReadMs = now;
    } else {
      // Sensor not ready, just update timestamp to avoid hammering
      lastReadMs = now;
    }
  }

  // Return the smoothed data
  if (!haveLastReading) {
    return false;
  }
  co2_out   = (uint16_t)filteredCO2;
  tempC_out = filteredTempC;
  hum_out   = filteredHumidity;
  return true;
}

bool SCD41Sensor::readMeasurementF(uint16_t &co2_out, float &tempC_out, float &tempF_out, float &hum_out) {
  if (!readMeasurement(co2_out, tempC_out, hum_out)) {
    return false;
  }
  tempF_out = tempC_out * 9.0f / 5.0f + 32.0f;
  return true;
}
