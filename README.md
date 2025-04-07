# 🌱 Advanced Air Quality Monitor (Work in Progress)

This project involves building an advanced portable air quality monitor using a Metro M4 Grand Central board, Arduino IDE, various environmental sensors, and future integration with machine learning.

## 📌 **Current Features:**
- **Metro M4 Grand Central board** (Adafruit)
- Custom-written C++ libraries for streamlined sensor integration:
  - **PMSA003I** (PM1.0, PM2.5, PM10)
  - **SCD-41** (CO₂, temperature [°F], humidity)
  - **SGP30** (VOCs, corrected using SCD-41 data for better accuracy)
  - **BMP280** (barometric pressure & altitude)

## 🔄 **Sensor Fusion & Data Calibration:**
Using **temperature and humidity** from the SCD-41 sensor, the accuracy of VOC measurements (SGP30) is enhanced significantly by humidity compensation. Additional atmospheric data from the BMP280 sensor aids in distinguishing environmental influences on air quality.

## 🚧 **Current Challenges:**
- Troubleshooting ongoing errors with hardware/software integration.
- Integration of an Adafruit display screen for real-time visualization of all sensor data.

## 🔮 **Upcoming Plans (by May 2025):**
- Integrate portable battery power.
- Design and 3D-print a custom enclosure.
- Predictive analytics for future weather and air quality trends using recorded sensor data.
- Implement machine learning for enhanced sensor fusion and anomaly detection.

## 💡 **Potential Future Enhancements:**
- Cloud-based monitoring and data logging.
- Standalone operation (no external computers required).

## 🛠 **Tools & Technologies:**
- Arduino IDE, C++
- Metro M4 Grand Central
- Raspberry Pi (planned for ML integration)

## 📆 **Project Timeline:**
- **Start:** January 2025
- **Expected Completion:** May 2025

## ⚠️ **Note:**
This project is actively under development, with regular updates expected.

## 📝 **License:**
MIT (recommended)
