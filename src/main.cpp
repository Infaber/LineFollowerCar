#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "PID.hpp"
#include "WifiPid.hpp"

#define LED 2

// ===== DEBUG FLAGS =====
#define DEBUG_SERIAL  true
#define DEBUG_PID     true
#define DEBUG_LOOP_HZ true

IRSensor irSensor;
Motordriver motors;

PID pid(0.03, 0.00, 0.10, IRSensor::CENTER_POSITION);
WifiPid wifi(pid, 0.03, 0.00, 0.10);

int baseSpeedValue = 100;
const int Max_Motor_speed = 150;

void setup() {
    Serial.begin(115200);
    delay(200);

    if (DEBUG_SERIAL) Serial.println("Booting...");

    wifi.begin();
    if (DEBUG_SERIAL) Serial.println("WiFi started");

    irSensor.calibrate(2000);
    if (DEBUG_SERIAL) Serial.println("Calibration done");
}

void loop() {
    wifi.handle();
/*
#if DEBUG_LOOP_HZ
    // ===== LOOP HZ DEBUG (1 gang per sekund) =====
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

    // ===== START/STOP: hvis stoppet -> hold motorer av =====
    if (!wifi.isRunning()) {
        motors.left_motor(0);
        motors.right_motor(0);
        return;
    }

    // Les posisjon fra IR sensor
    uint16_t position = irSensor.readPosition();

    // Beregn PID korreksjon
    float corrF = pid.compute(position);
    int correction = (int)roundf(corrF);

    // Slik at regulering ikke blir alt for aggressiv ved høye hastigheter
    int maxCorr = baseSpeedValue + 20;
    correction = constrain(correction, -maxCorr, maxCorr);

    // Kalkuler motor hastigheter
    int left  = baseSpeedValue - correction;
    int right = baseSpeedValue + correction;

    // Begrens motorhastighet
    left  = constrain(left, 0, Max_Motor_speed);
    right = constrain(right, 0, Max_Motor_speed);

    motors.left_motor(left);
    motors.right_motor(right);

#if DEBUG_PID
    // ===== PID/MOTOR DEBUG (2 ganger per sekund) =====
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