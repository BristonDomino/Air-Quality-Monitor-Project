#include <Wire.h>
#include "Adafruit_PM25AQI.h"
#include "SCD41Sensor.h"
#include "SGP30Sensor.h"
#include "BMP280Sensor.h"
#include "ScreenDisplay.h"

// Sensor objects
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
SCD41Sensor scdSensor;
SGP30Sensor sgpSensor;
ScreenDisplay screen;

// Create a BMP280 sensor object, optionally specifying the local sea-level pressure in hPa
BMP280Sensor bmpSensor(1013.25f);

// Variables to store previous readings
uint16_t prevCO2 = 0;
float prevTempC = 0.0;
float prevTempF = 0.0;
float prevHumidity = 0.0;

void setup() {

  Serial.begin(230400);
 // while (!Serial) delay(10);

  Serial.println("Initializing all sensors...");

  // PMSA003I
  if (!aqi.begin_I2C()) {
    Serial.println("PMSA003I initialization failed!");
    while (1) delay(10);
  }
  Serial.println("PMSA003I ready!");

  // SCD41
  if (!scdSensor.begin(3)) {
    Serial.println("SCD41 not connected, continuing without it.");
  }

  // SGP30
  if (!sgpSensor.begin(2)) {
    Serial.println("SGP30 not connected, continuing without it.");
  }

  // BMP280
  if (!bmpSensor.begin(0x77, 3)) {
    Serial.println("BMP280 isn't connected, but continuing anyway...");
  }

  // screen
  screen.begin();
  screen.startInitPage();
  Serial.println("Initializing all sensors");  // optional
  screen.addInitLine("Initializing all sensors");

  /* --- SCD41 --- */
  bool scdOK = scdSensor.begin(2);
  screen.addInitLine(scdOK ? "SCD41 OK" : "SCD41 FAIL");

  /* --- SGP30 --- */
  bool sgpOK = sgpSensor.begin(2);
  screen.addInitLine(sgpOK ? "SGP30 OK" : "SGP30 FAIL");

  /* --- BMP280 --- */
  bool bmpOK = bmpSensor.begin();
  screen.addInitLine(bmpOK ? "BMP280 OK" : "BMP280 FAIL");

  /* --- PMSA003I etc. (add your own) --- */
  screen.addInitLine("PMSA003I OK");

  screen.closeInitPage();  // 4-second pause before live page


  Serial.println("All sensors initialized successfully!");
}


void loop() {
  // 1) PMSA003I: just like your existing code
  PM25_AQI_Data data;
  if (aqi.read(&data)) {
    Serial.println("PMSA003I Results:");
    Serial.print(" PM1.0: ");
    Serial.println(data.pm10_standard);
    Serial.print(" PM2.5: ");
    Serial.println(data.pm25_standard);
    Serial.print(" PM10:  ");
    Serial.println(data.pm100_standard);
  }

  // 2) SCD41: read measurement with Fahrenheit
  uint16_t co2;
  float tempC, tempF, hum;
  if (scdSensor.connected()) {
    if (scdSensor.readMeasurementF(co2, tempC, tempF, hum)) {
      Serial.println("SCD41 Results (cached or fresh):");
      Serial.print(" CO2 (ppm):  ");
      Serial.println(co2);
      Serial.print(" Temp (F):   ");
      Serial.println(tempF);
      Serial.print(" Humidity:   ");
      Serial.println(hum);
    }
  }

  // 3) SGP30: humidity compensation, get VOC in both ppb and mg/m^3
  uint16_t voc_ppb;
  float voc_mg;
  if (sgpSensor.readCalibratedVOC(tempC, hum, voc_ppb, voc_mg)) {
    Serial.println("SGP30 Results (humidity-compensated):");
    Serial.print(" VOC (ppb):   ");
    Serial.println(voc_ppb);
    Serial.print(" VOC (mg/m^3): ");
    Serial.println(voc_mg, 3);  // 3 decimal places
  }

  // 4) BMP280: read data
  float pressure_inhg, alt_m, alt_ft;
  if (bmpSensor.connected()) {
    if (bmpSensor.readData(pressure_inhg, alt_m, alt_ft)) {
      Serial.println("BMP280 Results:");
      Serial.print(" Pressure (inHg): ");
      Serial.println(pressure_inhg, 4);
      Serial.print(" Alt (m):         ");
      Serial.println(alt_m, 2);
      Serial.print(" Alt (ft):        ");
      Serial.println(alt_ft, 2);
    }
  }

  SensorReadings sr;
  sr.pm1 = data.pm10_standard;    // PM 1.0
  sr.pm25 = data.pm25_standard;   // PM 2.5
  sr.pm10 = data.pm100_standard;  // PM 10
  sr.co2 = co2;
  sr.tempF = tempF;
  sr.humidity = hum;
  sr.voc_ppb = voc_ppb;
  sr.voc_mg = voc_mg;
  sr.pressure_inhg = pressure_inhg;
  sr.altitude_m = alt_m;  // metres from BMP280

  screen.render(sr);  // <-- one call, that’s all

  Serial.println("Version 0.12.0");
  Serial.println("------------------------------------");

  delay(2500);  // Read every 3 seconds
}