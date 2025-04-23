#include "ScreenDisplay.h"

bool ScreenDisplay::begin() {
    tft.begin(HX8357D);
    tft.setRotation(1);            // landscape
    tft.fillScreen(HX8357_BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.setCursor(50, 140);
    tft.println("Display ready");
    Serial.println("Display ready");
    return true;
}

void ScreenDisplay::printPair(int16_t x, int16_t& y,
                              const char* label, float val, uint8_t prec) {
    tft.setCursor(x, y);
    tft.print(label);
    tft.println(val, prec);
    y += 28;
}

void ScreenDisplay::render(const SensorReadings& s) {
    tft.fillScreen(HX8357_BLACK);

    int16_t x = 10;
    int16_t y = 10;
    tft.setCursor(x, y);
    tft.println("Sensor Readings");

    y += 30;
    printPair(x, y, "PM1.0  (ug/m3): ", s.pm1);
    printPair(x, y, "PM2.5  (ug/m3): ", s.pm25);
    printPair(x, y, "PM10   (ug/m3): ", s.pm10);
    printPair(x, y, "CO2    (ppm):   ", s.co2);
    printPair(x, y, "Temp   (F):     ", s.tempF, 1);
    printPair(x, y, "Humidity(%):    ", s.humidity, 1);

    /* right-hand column */
    x = 250; y = 40;
    printPair(x, y, "VOC  (ppb): ", s.voc_ppb);
    printPair(x, y, "VOC(mg/m3):", s.voc_mg, 3);
    printPair(x, y, "Press (inHg):", s.pressure_inhg, 3);
    printPair(x, y, "Alt   (m):   ", s.altitude_m, 0);
}
