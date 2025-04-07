#include "SCD41Sensor.h"

SCD41Sensor::SCD41Sensor() : scd4x() {}

bool SCD41Sensor::begin() {
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire, 0x62); // Default I2C address

  // Stop previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error stopping periodic measurement: ");
    errorToString(error, errorMessage, sizeof(errorMessage));
    Serial.println(errorMessage);
    return false;
  }

  delay(500);

  // Start periodic measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error starting periodic measurement: ");
    errorToString(error, errorMessage, sizeof(errorMessage));
    Serial.println(errorMessage);
    return false;
  }

  Serial.println("SCD-41 sensor initialized successfully!");
  return true;
}

// Helper to apply exponential smoothing
float SCD41Sensor::smoothValue(float currentFiltered, float newRaw) {
  return currentFiltered + alpha * (newRaw - currentFiltered);
}

bool SCD41Sensor::readMeasurement(uint16_t &co2_out, float &tempC_out, float &hum_out) {
  unsigned long now = millis();
  bool doFreshRead = (now - lastReadMs >= readInterval);

  if (doFreshRead) {
    // Attempt a fresh read if enough time has passed
    uint16_t error;
    char errorMessage[256];
    bool dataReady = false;

    error = scd4x.getDataReadyStatus(dataReady);
    if (error) {
      Serial.print("Error reading data ready status: ");
      errorToString(error, errorMessage, sizeof(errorMessage));
      Serial.println(errorMessage);
      if (!haveLastReading) return false;
      // fallback to filtered
      co2_out    = (uint16_t)filteredCO2;
      tempC_out  = filteredTempC;
      hum_out    = filteredHumidity;
      return true;
    }

    if (dataReady) {
      uint16_t rawCO2;
      float rawTempC, rawHum;

      error = scd4x.readMeasurement(rawCO2, rawTempC, rawHum);
      if (error) {
        Serial.print("Error reading measurement: ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
        if (!haveLastReading) return false;
        // fallback
        co2_out    = (uint16_t)filteredCO2;
        tempC_out  = filteredTempC;
        hum_out    = filteredHumidity;
        return true;
      }
      // We got fresh data, apply smoothing
      if (!haveLastReading) {
        // First reading: initialize filtered to raw
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
      // No new sensor data, do nothing special except update lastReadMs
      lastReadMs = now;
    }
  }

  // Return the (possibly updated) filtered data
  if (!haveLastReading) {
    return false; // never had a valid read
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
  // Convert the *filtered* Celsius to Fahrenheit
  tempF_out = tempC_out * 9.0f / 5.0f + 32.0f;
  return true;
}
