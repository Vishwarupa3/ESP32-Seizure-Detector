# ESP32-Seizure-Detector
This project is a wearable Seizure Detection and Alert System built using ESP32, designed to detect abnormal physiological parameters and movement patterns associated with seizures. It uses real-time SpO2, Heart Rate, Accelerometer, and Vibration sensor data to identify possible seizure events and immediately trigger alerts via Blynk IoT.

# Components Used:
- ESP32 Dev Board
- MAX30100 (Heart Rate and SpO2 sensor)
- MPU6050 (Accelerometer + Gyro)
- Vibration Sensor
- Buzzer
- Panic/Reset Button

## 📲 Features
- Real-time monitoring of SpO2 and Heart Rate
- Seizure-like movement detection using accelerometer and vibration sensor
- Automatic alerts via Blynk
- Buzzer alarm with manual reset button

## 🔗 Blynk Integration
The system uses Blynk for remote monitoring. You need to configure:
- `BLYNK_TEMPLATE_ID`
- `BLYNK_TEMPLATE_NAME`
- `BLYNK_AUTH_TOKEN`

## 📁 Files Included
- `SeizureAlertDevice.ino`: Main Arduino sketch
- `README.md`: Project documentation

## ⚠️ License
Open-sourced under the MIT License.

