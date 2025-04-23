#include "SGP30Sensor.h"

/* ====================================================================== */
/* -----------------------  helper functions  --------------------------- */
/* ====================================================================== */

uint16_t SGP30Sensor::computeAbsoluteHumidity(float tC, float rh) {
    /* Sensirion AH approximation (mg/m³) → scaled for SGP30 (×256) */
    float tK      = tC + 273.15f;
    float expTerm = exp((17.62f * tC) / (243.12f + tC));
    float pws     = 6.112f * expTerm;                 // sat. vapour pressure (mbar)
    float ah_gm3  = 216.7f * (rh / 100.0f) * pws / tK;
    return static_cast<uint16_t>(ah_gm3 * 1000.0f * 256.0f);
}

/* ---------------------------------------------------------------------- */
bool SGP30Sensor::initSD() {
    if (SD.begin(83)) {                               // CS = 10 on Metro M4 GC
        Serial.println("SD card mounted.");
        return true;
    }
    Serial.println("⚠️  SD card mount failed – baseline will not persist.");
    return false;
}

/* ---------------------------------------------------------------------- */
bool SGP30Sensor::loadBaseline(uint16_t &eco2, uint16_t &tvoc) {
    File f = SD.open(BASELINE_FILE, FILE_READ);
    if (!f) return false;
    String line = f.readStringUntil('\n');
    f.close();
    return sscanf(line.c_str(), "%hu %hu", &eco2, &tvoc) == 2;
}

/* ---------------------------------------------------------------------- */
void SGP30Sensor::saveBaseline(uint16_t eco2, uint16_t tvoc) {
    File f = SD.open(BASELINE_FILE, FILE_WRITE);
    if (!f) {
        Serial.println("⚠️  Could not open baseline file for write.");
        return;
    }
    f.seek(0);                         // overwrite
    f.printf("%u %u\n", eco2, tvoc);
    f.close();
    Serial.printf("Baseline saved  eCO2=%u  TVOC=%u\n", eco2, tvoc);
}

/* ====================================================================== */
/* -----------------------------  PUBLIC  --------------------------------*/
/* ====================================================================== */
bool SGP30Sensor::begin(int maxRetries) {
    bool sdOK = initSD();              // try to mount SD first (non‑fatal)

    for (int i = 1; i <= maxRetries && !isConnected; ++i) {
        Serial.printf("SGP30 init attempt %d/%d…\n", i, maxRetries);
        if (sgp30.begin()) {
            Serial.println("SGP30 online.");
            sgp30.IAQinit();           // start learning baseline
            isConnected = true;

            /* ----------  baseline restore if present  ---------- */
            uint16_t eco2B, tvocB;
            if (sdOK && loadBaseline(eco2B, tvocB)) {
                sgp30.setIAQBaseline(eco2B, tvocB);
                Serial.printf("Baseline applied  eCO2=%u  TVOC=%u\n", eco2B, tvocB);
            }
        } else {
            delay(250);
        }
    }
    return isConnected;
}

/* ---------------------------------------------------------------------- */
bool SGP30Sensor::readVOC(uint16_t &voc_ppb) {
    if (!isConnected)               return false;
    if (!sgp30.IAQmeasure())        return false;
    voc_ppb = sgp30.TVOC;
    return true;
}

/* ---------------------------------------------------------------------- */
bool SGP30Sensor::readCalibratedVOC(float tC, float rh,
                                    uint16_t &voc_ppb_out, float &voc_mg_out) {
    if (!isConnected) return false;

    unsigned long now = millis();
    bool doFresh = (now - lastReadMs >= readInterval);

    if (doFresh) {
        /* ---- humidity compensation ---- */
        sgp30.setHumidity(computeAbsoluteHumidity(tC, rh));

        /* ---- new measurement ---- */
        if (!sgp30.IAQmeasure()) return haveLastReading;
        uint16_t raw_ppb = sgp30.TVOC;
        float    raw_mg  = raw_ppb * 0.00409f;   // MW≈100 g/mol

        /* ---- smoothing ---- */
        if (!haveLastReading) {
            filteredVOC_ppb = raw_ppb;
            filteredVOC_mg  = raw_mg;
            haveLastReading = true;
        } else {
            filteredVOC_ppb = smoothValue(filteredVOC_ppb, raw_ppb);
            filteredVOC_mg  = smoothValue(filteredVOC_mg,  raw_mg);
        }
        lastReadMs = now;

        /* ---- periodic baseline save (1 h) ---- */
        if (now - lastBaselineSaveMs >= BASELINE_SAVE_INTERVAL) {
            uint16_t eco2Base, tvocBase;
            if (sgp30.getIAQBaseline(&eco2Base, &tvocBase)) {
                saveBaseline(eco2Base, tvocBase);
            }
            lastBaselineSaveMs = now;
        }
    }

    if (!haveLastReading) return false;
    voc_ppb_out = static_cast<uint16_t>(filteredVOC_ppb);
    voc_mg_out  = filteredVOC_mg;
    return true;
}
