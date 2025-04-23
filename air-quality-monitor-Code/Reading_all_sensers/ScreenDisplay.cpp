#include "ScreenDisplay.h"

bool ScreenDisplay::begin() {
    tft.begin(HX8357D);
    tft.setRotation(1);
    tft.fillScreen(HX8357_BLACK);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    return true;
}

/* ---------------- boot-time page ---------------- */
void ScreenDisplay::startInitPage() {
    tft.fillScreen(HX8357_BLACK);
    tft.setCursor(10, 5);
    tft.println("Initializing …");
    initY = 30;
}
void ScreenDisplay::addInitLine(const char* m) {
    tft.setCursor(10, initY);
    tft.println(m);
    initY += 24;
}
void ScreenDisplay::closeInitPage() {
    delay(4000);
}

/* ---------------- live page -------------------- */
void ScreenDisplay::printPair(int16_t x,int16_t& y,
                              const char* lbl,float v,uint8_t p){
    tft.setCursor(x,y); tft.print(lbl); tft.println(v,p); y+=28;
}

void ScreenDisplay::render(const SensorReadings& s) {
    tft.fillScreen(HX8357_BLACK);
    int16_t x=10,y=10;
    tft.setCursor(x,y); tft.println("Sensor Readings"); y+=30;
    printPair(x,y,"PM1.0 (ug): ",s.pm1);
    printPair(x,y,"PM2.5 (ug): ",s.pm25);
    printPair(x,y,"PM10  (ug): ",s.pm10);
    printPair(x,y,"CO2  (ppm): ",s.co2);
    printPair(x,y,"Temp   F  : ",s.tempF,1);
    printPair(x,y,"Humidity %: ",s.humidity,1);
    x=250; y=40;
    printPair(x,y,"VOC  ppb : ",s.voc_ppb);
    printPair(x,y,"VOC mg/m3: ",s.voc_mg,3);
    printPair(x,y,"Press inHg: ",s.pressure_inhg,3);
    printPair(x,y,"Alt   (m): ",s.altitude_m,0);
}
