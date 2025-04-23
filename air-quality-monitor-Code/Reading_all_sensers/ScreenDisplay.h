#ifndef SCREENDISPLAY_H
#define SCREENDISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>

/* -------- a simple struct to pass all values at once -------- */
struct SensorReadings {
    float     pm1;
    float     pm25;
    float     pm10;
    uint16_t  co2;
    float     tempF;
    float     humidity;
    uint16_t  voc_ppb;
    float     voc_mg;
    float     pressure_inhg;
    float     altitude_m; 
};

class ScreenDisplay {
public:
    bool begin();                                  // call once in setup()
    void render(const SensorReadings& s);          // call whenever you have new data

private:
    /* FeatherWing HX8357 pins for the Metro M4 Grand Central */
    static constexpr uint8_t TFT_CS  = 9;
    static constexpr uint8_t TFT_DC  = 10;
    static constexpr int8_t  TFT_RST = -1;         // tied to reset

    Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

    /* simple text helper */
    void printPair(int16_t x, int16_t& y, const char* label, float value, uint8_t prec = 0);
};

#endif
