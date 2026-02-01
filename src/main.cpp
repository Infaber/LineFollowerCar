#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "PID.hpp"
#include "WifiPid.hpp"

#define LED 2

// Debug
const bool DEBUG_SERIAL = true;
const bool DEBUG_PID    = true;

// Sensor + motor
IRSensor irSensor;
Motordriver motors;

// Svingefølsomhet via WiFi
float turnGain = 1.00f;

// PID startverdier (tunes via WiFi)
PID pid(0.065, 0.00, 0.22, IRSensor::CENTER_POSITION);

// Fart
int baseSpeedValue = 150;
const int MAX_PWM = 255;

// NYTT: hindrer at den snapper hardt til siden
const int MAX_TURN = 225;     // juster 50–120

// NYTT: hindrer at den jager rundt midten pga støy
const int DEADBAND = 15;     // juster 15–60

WifiPid wifi(pid, irSensor, baseSpeedValue, turnGain, 0.065, 0.00, 0.22);

void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    Serial.begin(115200);
    delay(200);

    if (DEBUG_SERIAL) Serial.println("Booting...");

    wifi.begin();
    if (DEBUG_SERIAL) Serial.println("WiFi started");

    if (DEBUG_SERIAL) Serial.println("Starting calibration...");
    irSensor.calibrate(2000);
    if (DEBUG_SERIAL) Serial.println("Calibration done");

    // wifi.start();  // valgfritt auto-start
}

void loop() {
    wifi.handle();

    // STOPP hvis ikke RUN
    if (!wifi.isRunning()) {
        motors.left_motor(0);
        motors.right_motor(0);
        digitalWrite(LED, LOW);
        return;
    }

    digitalWrite(LED, HIGH);

    // 1) Les posisjon
    uint16_t position = irSensor.readPosition();

    // 2) PID -> motorSpeed (skaleres med turnGain)
    float raw = pid.compute(position);
    int motorSpeed = (int)lroundf(raw * turnGain);

    // ===== NYTT: Deadband rundt midten =====
    // Hvis du er "nærme nok" midten, ikke korriger i det hele tatt.
    int error = (int)position - (int)IRSensor::CENTER_POSITION;
    if (abs(error) < DEADBAND) {
        motorSpeed = 0;
    }

    // ===== NYTT: Maks sving =====
    // Hindrer at den svinger alt for hardt og begynner å oscillere.
    motorSpeed = constrain(motorSpeed, -MAX_TURN, MAX_TURN);

    // 3) Motor-miks (denne retningen var riktig for deg)
    int right = baseSpeedValue - motorSpeed;
    int left  = baseSpeedValue + motorSpeed;

    // 4) Full range + revers
    right = constrain(right, -MAX_PWM, MAX_PWM);
    left  = constrain(left,  -MAX_PWM, MAX_PWM);

    // 5) Kjør motorer
    motors.right_motor(right);
    motors.left_motor(left);

    // Debug
    if (DEBUG_PID) {
        static unsigned long lastDebug = 0;
        if (millis() - lastDebug > 300) {
            Serial.print("Pos: "); Serial.print(position);
            Serial.print(" | Err: "); Serial.print(error);
            Serial.print(" | PID: "); Serial.print(raw, 3);
            Serial.print(" | Gain: "); Serial.print(turnGain, 2);
            Serial.print(" | MS: "); Serial.print(motorSpeed);
            Serial.print(" | L: "); Serial.print(left);
            Serial.print(" | R: "); Serial.print(right);
            Serial.print(" | Base: "); Serial.print(baseSpeedValue);
            Serial.println();
            lastDebug = millis();
        }
    }
}
