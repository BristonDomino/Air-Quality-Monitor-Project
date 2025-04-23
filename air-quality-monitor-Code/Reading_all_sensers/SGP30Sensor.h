#ifndef SGP30SENSOR_H
#define SGP30SENSOR_H

#include <Arduino.h>
#include <Adafruit_SGP30.h>
#include <SD.h>           // µSD baseline storage
#include <SPI.h>

/* ------------- class ------------- */
class SGP30Sensor {
private:
    /* ---- hardware ---- */
    Adafruit_SGP30 sgp30;
    bool   isConnected = false;

    /* ---- baseline persistence ---- */
    static constexpr const char*  BASELINE_FILE = "/sgp30_baseline.txt";
    static constexpr unsigned long BASELINE_SAVE_INTERVAL = 3600000UL;   // 1 hour
    unsigned long lastBaselineSaveMs = 0;

    /* ---- humidity compensation ---- */
    uint16_t computeAbsoluteHumidity(float tC, float rhPercent);

    /* ---- simple exponential smoothing for VOCs ---- */
    bool   haveLastReading   = false;
    float  filteredVOC_ppb   = 0.0f;
    float  filteredVOC_mg    = 0.0f;
    unsigned long lastReadMs = 0;
    static constexpr unsigned long readInterval = 2000UL;  // 2 s update window
    static constexpr float  alpha = 0.30f;                 // smoothing factor
    float  smoothValue(float cur, float raw) { return cur + alpha * (raw - cur); }

    /* ---- SD helpers ---- */
    bool  initSD();                                                // mount card
    bool  loadBaseline(uint16_t &eco2, uint16_t &tvoc);            // file → RAM
    void  saveBaseline(uint16_t eco2, uint16_t tvoc);              // RAM → file

public:
    /* ---- API ---- */
    SGP30Sensor() = default;
    bool  begin(int maxRetries = 2);                // mounts SD, loads baseline
    bool  connected() const { return isConnected; }

    bool  readVOC(uint16_t &voc_ppb);               // no humidity comp
    bool  readCalibratedVOC(float tC, float rh,
                            uint16_t &voc_ppb, float &voc_mg); // hum‑comp + save
};

#endif
