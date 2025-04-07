#include "SCD41Sensor.h"

SCD41Sensor::SCD41Sensor() : scd4x() {}

bool SCD41Sensor::begin() {
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire, 0x62); // default I2C address

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
  uint16_t error;
  char errorMessage[256];
  bool dataReady = false;

  error = scd4x.getDataReadyStatus(dataReady);
  if (error) {
    Serial.print("Error reading data ready status: ");
    errorToString(error, errorMessage, sizeof(errorMessage));
    Serial.println(errorMessage);
    // If we can’t even check data status, return false *only if* we never had a valid reading before
    if (!haveLastReading) return false;
    // Otherwise, fall back to last reading
    co2 = lastCO2;
    temperatureC = lastTempC;
    humidity = lastHumidity;
    return true;
  }

  if (dataReady) {
    // We have fresh data, read it
    error = scd4x.readMeasurement(co2, temperatureC, humidity);
    if (error) {
      Serial.print("Error reading measurement: ");
      errorToString(error, errorMessage, sizeof(errorMessage));
      Serial.println(errorMessage);
      // Return false if never had a good reading before
      if (!haveLastReading) return false;
      // Otherwise, fall back to last reading
      co2 = lastCO2;
      temperatureC = lastTempC;
      humidity = lastHumidity;
      return true;
    }
    // Update our cached values
    lastCO2 = co2;
    lastTempC = temperatureC;
    lastHumidity = humidity;
    haveLastReading = true;
  } else {
    // No new data — fallback to the last known reading
    if (!haveLastReading) {
      // We have never successfully read data before
      return false;
    }
    co2 = lastCO2;
    temperatureC = lastTempC;
    humidity = lastHumidity;
  }
  return true;
}

// readMeasurementF: same logic, but also calculates Fahrenheit
bool SCD41Sensor::readMeasurementF(uint16_t &co2, float &temperatureC, float &temperatureF, float &humidity) {
  // Reuse the logic from readMeasurement
  bool success = readMeasurement(co2, temperatureC, humidity);
  if (!success) return false;

  temperatureF = temperatureC * 9.0f / 5.0f + 32.0f;
  return true;
}
