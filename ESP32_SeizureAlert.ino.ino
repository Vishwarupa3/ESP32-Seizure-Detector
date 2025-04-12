#define BLYNK_TEMPLATE_ID "TMPL6e1abcdef"
#define BLYNK_TEMPLATE_NAME "SeizureAlertDevice"
#define BLYNK_AUTH_TOKEN "djrHgY_PPz6IhummP03iMCzzuzwSA1ri"


#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <MPU6050.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Tu tera le";
char pass[] = "girijajoshi";

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

    // WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");

    // Blynk Initialization
    Blynk.config(BLYNK_AUTH_TOKEN);
    if (Blynk.connect()) {
        Serial.println("Blynk connected!");
    } else {
        Serial.println("Failed to connect to Blynk!");
    }

    // MAX30100
    Serial.println("Initializing MAX30100...");
    if (!pox.begin()) {
        Serial.println("MAX30100 initialization failed!");
        while (1);
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);

    // MPU6050
    Serial.println("Initializing MPU6050...");
    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("MPU6050 initialization failed!");
        while (1);
    }

    // Pins
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
        Serial.println("Invalid MAX30100 data. Skipping.");
        return;
    }

    int vibration = digitalRead(VIBRATION_PIN);

    checkSensors(spo2, heartRate, ax, ay, az, vibration);

    if (millis() - tsLastReport > 1000) {
        tsLastReport = millis();
        float movement = sqrt((ax * ax) + (ay * ay) + (az * az));

        Serial.printf("SpO2: %.2f%% | HR: %.2f bpm | Movement: %.2f | Vibration: %d\n",
                      spo2, heartRate, movement, vibration);

        // Send to Blynk
        Blynk.virtualWrite(V0, spo2);
        Blynk.virtualWrite(V1, heartRate);
        Blynk.virtualWrite(V2, movement);
        Blynk.virtualWrite(V3, vibration);
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
                triggerAlarm("Seizure-Like Movement Detected!");
            }
        }
    }
}

void triggerAlarm(const char *reason) {
    alarmTriggered = true;
    Serial.println(reason);
    digitalWrite(BUZZER_PIN, HIGH);
    Blynk.virtualWrite(V4, reason);
}

void resetSystem() {
    alarmTriggered = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("System reset.");
    Blynk.virtualWrite(V4, "System Reset");
}
