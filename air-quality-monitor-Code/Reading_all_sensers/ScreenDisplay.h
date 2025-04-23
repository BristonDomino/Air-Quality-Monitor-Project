#ifndef SCREENDISPLAY_H
#define SCREENDISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>

/* ---- compact data bucket for live page ---- */
struct SensorReadings {
    float     pm1 , pm25 , pm10;
    uint16_t  co2;
    float     tempF , humidity;
    uint16_t  voc_ppb;  float voc_mg;
    float     pressure_inhg , altitude_m;
};

class ScreenDisplay {
public:
    bool begin();                                        // call once
    /* boot-time init log */
    void startInitPage();                                // clear & title
    void addInitLine(const char* msg);                   // append line
    void closeInitPage();                                // 3s pause

    /* live page */
    void render(const SensorReadings& s);

private:
    /* pins for FeatherWing on Metro M4 GC */
    static constexpr uint8_t TFT_CS  = 9;
    static constexpr uint8_t TFT_DC  = 10;
    static constexpr int8_t  TFT_RST = -1;

    Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

    /* helpers */
    int16_t initY = 30;
    void    printPair(int16_t x, int16_t& y,
                      const char* label, float v, uint8_t prec = 0);
};

#endif
