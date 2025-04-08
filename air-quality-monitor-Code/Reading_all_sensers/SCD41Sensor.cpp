#include "SCD41Sensor.h"

// Helper that toggles SCL ~9 times to release a stuck SDA
static void i2cBusRecoveryPins(uint8_t sclPin, uint8_t sdaPin) {
  pinMode(sclPin, OUTPUT);
  for (int i = 0; i < 9; i++) {
    digitalWrite(sclPin, LOW);
    delayMicroseconds(5);
    digitalWrite(sclPin, HIGH);
    delayMicroseconds(5);
  }
  // Let’s release the lines (back to input/pull-ups)
  pinMode(sclPin, INPUT_PULLUP);
  pinMode(sdaPin, INPUT_PULLUP);
}

SCD41Sensor::SCD41Sensor() : scd4x() {}

bool SCD41Sensor::begin(int maxRetries) {
  Wire.begin(); 
  isConnected = false;
  consecutiveErrorCount = 0; // reset at initialization

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
      delay(200);
      continue; 
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

    // Success
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

      // Count an error
      consecutiveErrorCount++;
      if (consecutiveErrorCount >= maxConsecutiveErrors) {
        Serial.println("SCD41: Too many consecutive errors, attempting I2C recovery...");
        recoverI2CBus();
      }

      // fallback if we have one
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

        // Count error
        consecutiveErrorCount++;
        if (consecutiveErrorCount >= maxConsecutiveErrors) {
          Serial.println("SCD41: Too many consecutive errors, attempting I2C recovery...");
          recoverI2CBus();
        }

        if (!haveLastReading) return false;
        co2_out   = (uint16_t)filteredCO2;
        tempC_out = filteredTempC;
        hum_out   = filteredHumidity;
        return true;
      }

      // Fresh data! reset error count
      consecutiveErrorCount = 0; 

      if (!haveLastReading) {
        filteredCO2      = (float)rawCO2;
        filteredTempC    = rawTempC;
        filteredHumidity = rawHum;
        haveLastReading  = true;
      } else {
        filteredCO2      = smoothValue(filteredCO2, (float)rawCO2);
        filteredTempC    = smoothValue(filteredTempC, rawTempC);
        filteredHumidity = smoothValue(filteredHumidity, rawHum);
      }
      lastReadMs = now;
    } else {
      // No new data, but no error. reset error count
      consecutiveErrorCount = 0;
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

// Attempts to free the I2C bus and re-init the sensor
void SCD41Sensor::recoverI2CBus() {
  Serial.println("SCD41: Entering recoverI2CBus()...");

  // 1) End any existing Wire session
  Wire.end();

  // 2) Attempt bus recovery pulses
  // For the Metro M4 Grand Central:
  //   SDA = Pin 20, SCL = Pin 21
  // Adjust if you moved them, e.g. SCL1, etc.
  const uint8_t sclPin = 21;
  const uint8_t sdaPin = 20;

  i2cBusRecoveryPins(sclPin, sdaPin);

  // 3) Restart I2C
  Wire.begin();

  // 4) Attempt to re-begin sensor with 1 retry
  bool result = begin(1);
  if (result) {
    Serial.println("SCD41: Recovered I2C bus & re-initialized sensor!");
  } else {
    Serial.println("SCD41: I2C recovery attempt failed. Sensor remains disconnected.");
  }

  // Reset the error count to avoid spamming recovery attempts
  consecutiveErrorCount = 0;
}
