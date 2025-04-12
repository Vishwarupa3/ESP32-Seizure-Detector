#define BLYNK_TEMPLATE_ID "TMPL6e1abcdef"
#define BLYNK_TEMPLATE_NAME "SeizureAlertDevice"
#define BLYNK_AUTH_TOKEN "djrHgY_PPz6IhummP03iMCzzuzwSA1ri"

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <MPU6050.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "Tu tera le";
char pass[] = "girijajoshi";

// ESP32 Pin Assignments
#define SDA_PIN 21
#define SCL_PIN 22
#define VIBRATION_PIN 25
#define BUZZER_PIN 27
#define RESET_BUTTON_PIN 32

#define HEART_RATE_THRESHOLD 120
#define SPO2_THRESHOLD 90
#define MOVEMENT_THRESHOLD 15000

PulseOximeter pox;
MPU6050 mpu;
bool alarmTriggered = false;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat detected!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing System...");

  Wire.begin(SDA_PIN, SCL_PIN);

  // Blynk Init
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // MAX30100 Init
  Serial.println("Initializing MAX30100...");
  if (!pox.begin()) {
    Serial.println("MAX30100 initialization failed. Check connections!");
    while (1);
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // MPU6050 Init
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed. Check wiring!");
    while (1);
  }

  pinMode(VIBRATION_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  Blynk.run();
  pox.update();

  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    resetSystem();
  }

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float spo2 = pox.getSpO2();
  float heartRate = pox.getHeartRate();

  if (isnan(spo2) || spo2 <= 0 || isnan(heartRate) || heartRate <= 0) {
    Serial.println("Invalid MAX30100 data.");
    return;
  }

  int vibration = digitalRead(VIBRATION_PIN);

  // Check for conditions
  checkSensors(spo2, heartRate, ax, ay, az, vibration);

  if (millis() - tsLastReport > 1000) {
    tsLastReport = millis();

    // Compute magnitude of movement vector
    float movement = sqrt((ax * ax) + (ay * ay) + (az * az));

    Serial.println("------------- Sensor Data -------------");
    Serial.printf("SpO2: %.2f%% | Heart Rate: %.2f bpm\n", spo2, heartRate);
    Serial.printf("Movement: %.2f\n", movement);
    Serial.printf("Vibration: %s\n", (vibration == HIGH) ? "Detected" : "None");
    Serial.println("---------------------------------------");

    // Send real-time values to Blynk
    Blynk.virtualWrite(V0, spo2);       // SpO2 (%)
    Blynk.virtualWrite(V1, heartRate);  // Heart rate (bpm)
    Blynk.virtualWrite(V2, movement);   // Movement magnitude
    Blynk.virtualWrite(V3, vibration);  // Vibration: 1 or 0
  }
}

void checkSensors(float spo2, float heartRate, int16_t ax, int16_t ay, int16_t az, int vibration) {
  if (!alarmTriggered) {
    if (spo2 < SPO2_THRESHOLD) {
      triggerAlarm("Low SpO2 Detected!");
    } else if (heartRate > HEART_RATE_THRESHOLD) {
      triggerAlarm("High Heart Rate!");
    } else if (abs(ax) > MOVEMENT_THRESHOLD || abs(ay) > MOVEMENT_THRESHOLD || abs(az) > MOVEMENT_THRESHOLD) {
      if (vibration == HIGH) {
        triggerAlarm("Seizure-Like Movement!");
      }
    }
  }
}

void triggerAlarm(const char *reason) {
  alarmTriggered = true;
  Serial.println(reason);
  digitalWrite(BUZZER_PIN, HIGH);
  Blynk.virtualWrite(V4, reason);
  Blynk.logEvent("seizure_alert", reason);  // Optional Blynk notification
}

void resetSystem() {
  alarmTriggered = false;
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("System reset.");
  Blynk.virtualWrite(V4, "System Reset");
}