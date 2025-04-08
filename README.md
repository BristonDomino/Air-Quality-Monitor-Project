# 🌱 Advanced Air Quality Monitor (Work in Progress)

This project involves building an advanced portable air quality monitor using a Metro M4 Grand Central board, Arduino IDE, various environmental sensors, and future integration with machine learning.

## 📸 Project Images

### Sensor Wiring Setup
![Sensor Wiring](https://github.com/BristonDomino/Air-Quality-Monitor-Project/blob/main/All%20of%20the%20current%20wiring%20and%20board.jpg?raw=true)

### Arduine IDE and Output of Sensors
![Arduine IDE and Output of Sensors](https://github.com/BristonDomino/Air-Quality-Monitor-Project/blob/main/Arduine%20IDE%20and%20Output%20of%20Sensors%20.png)

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

## 🎥 Demo Video
[![Watch the demo](https://i.ytimg.com/vi/0X3jwfKh4xQ/oardefault.jpg?sqp=-oaymwEoCJUDENAFSFqQAgHyq4qpAxcIARUAAIhC2AEB4gEKCBgQAhgGOAFAAQ==&rs=AOn4CLDY-LD2IuJT5zJfDD_5cZTqQ5F7gA)](https://youtube.com/shorts/0X3jwfKh4xQ?feature=share)

