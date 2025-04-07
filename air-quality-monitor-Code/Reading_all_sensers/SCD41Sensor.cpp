#include "SCD41Sensor.h"

SCD41Sensor::SCD41Sensor() : scd4x() {}

bool SCD41Sensor::begin() {
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  // Default I2C address is 0x62
  scd4x.begin(Wire, 0x62);

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

bool SCD41Sensor::readMeasurement(uint16_t &co2, float &temperatureC, float &humidity) {
  // 1) Check if we can do a fresh read based on timing
  unsigned long now = millis();
  bool doFreshRead = (now - lastReadMs >= readInterval);

  if (doFreshRead) {
    // Attempt a fresh read if enough time has elapsed
    uint16_t error;
    char errorMessage[256];
    bool dataReady = false;

    // Query the sensor if new data is actually ready
    error = scd4x.getDataReadyStatus(dataReady);
    if (error) {
      Serial.print("Error reading data ready status: ");
      errorToString(error, errorMessage, sizeof(errorMessage));
      Serial.println(errorMessage);
      // If we have never had a valid reading, fail. Else, return the last reading
      if (!haveLastReading) return false;
      co2 = lastCO2;
      temperatureC = lastTempC;
      humidity = lastHumidity;
      return true;
    }

    if (dataReady) {
      // We have fresh data on the sensor
      error = scd4x.readMeasurement(co2, temperatureC, humidity);
      if (error) {
        Serial.print("Error reading measurement: ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
        // If never had a good reading, fail
        if (!haveLastReading) return false;
        // Otherwise return the cached data
        co2 = lastCO2;
        temperatureC = lastTempC;
        humidity = lastHumidity;
        return true;
      }
      // Successfully read, update cache
      lastCO2 = co2;
      lastTempC = temperatureC;
      lastHumidity = humidity;
      haveLastReading = true;
      lastReadMs = now; // Update timestamp
    } else {
      // The sensor isn't ready with new data
      // Return the cached data if we have it
      if (!haveLastReading) {
        return false; 
      }
      co2 = lastCO2;
      temperatureC = lastTempC;
      humidity = lastHumidity;
      // Update timestamp anyway to avoid hammering the sensor again
      lastReadMs = now;
    }
  } else {
    // 2) We haven't reached our 5-second interval yet
    // Return the cached data
    if (!haveLastReading) {
      // No reading yet to give
      return false;
    }
    co2 = lastCO2;
    temperatureC = lastTempC;
    humidity = lastHumidity;
  }

  return true;
}

bool SCD41Sensor::readMeasurementF(uint16_t &co2, float &temperatureC, float &temperatureF, float &humidity) {
  if (!readMeasurement(co2, temperatureC, humidity)) {
    return false;
  }
  temperatureF = temperatureC * 9.0f / 5.0f + 32.0f;
  return true;
}
