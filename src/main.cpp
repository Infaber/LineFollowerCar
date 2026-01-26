#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "PID.hpp"
#include "WifiPid.hpp"

#define LED 2

#define DEBUG_SERIAL  true
#define DEBUG_PID     true
#define DEBUG_LOOP_HZ true

IRSensor irSensor;
Motordriver motors;

PID pid(0.03, 0.00, 0.10, IRSensor::CENTER_POSITION);

int baseSpeedValue = 100;
const int Max_Motor_speed = 150;

// WiFi igjen
WifiPid wifi(pid, irSensor, baseSpeedValue, 0.03, 0.00, 0.10);

void setup() {
    Serial.begin(115200);
    delay(200);

    if (DEBUG_SERIAL) Serial.println("Booting...");

    wifi.begin();
    if (DEBUG_SERIAL) Serial.println("WiFi started");

    // Valgfritt: du kan fjerne denne og bare bruke CALIBRATE-knappen
    irSensor.calibrate(2000);
    if (DEBUG_SERIAL) Serial.println("Calibration done");
}

void loop() {
    wifi.handle();  // viktig
/*
#if DEBUG_LOOP_HZ
    static unsigned long lastHzTime = 0;
    static uint32_t loopCounter = 0;
    loopCounter++;
    if (millis() - lastHzTime >= 1000) {
        Serial.print("Loop Hz: ");
        Serial.println(loopCounter);
        loopCounter = 0;
        lastHzTime = millis();
    }
#endif
*/

    if (!wifi.isRunning()) {
        motors.left_motor(0);
        motors.right_motor(0);
        return;
    }

    uint16_t position = irSensor.readPosition();

    float corrF = pid.compute(position);
    int correction = (int)roundf(corrF);

    int maxCorr = baseSpeedValue + 20;
    correction = constrain(correction, -maxCorr, maxCorr);

    int left  = baseSpeedValue - correction;
    int right = baseSpeedValue + correction;

    left  = constrain(left, 0, Max_Motor_speed);
    right = constrain(right, 0, Max_Motor_speed);

    motors.left_motor(left);
    motors.right_motor(right);

#if DEBUG_PID
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.print("Pos: "); Serial.print(position);
        Serial.print(" | Corr: "); Serial.print(correction);
        Serial.print(" | L: "); Serial.print(left);
        Serial.print(" | R: "); Serial.print(right);
        Serial.print(" | Base: "); Serial.print(baseSpeedValue);
        Serial.print(" | RUN: "); Serial.print(wifi.isRunning() ? "1" : "0");
        Serial.println();
        lastDebug = millis();
    }
#endif
}