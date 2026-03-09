#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "MotorController.hpp"
#include "PID.hpp"
#include "WifiPid.hpp"

// ============ Configuration ============
constexpr uint8_t LED_PIN = 2;

// Debug flags
constexpr bool DEBUG_SERIAL = true;
constexpr bool DEBUG_PID    = true;

// PID default values
constexpr float DEFAULT_KP = 0.0280f;
constexpr float DEFAULT_KI = 0.0001f;
constexpr float DEFAULT_KD = 0.1198f;

// Motor settings
constexpr int DEFAULT_BASE_SPEED = 130;
constexpr int MAX_TURN = 225;
constexpr int MAX_PWM  = 255;

// ============ Global Objects ============
IRSensor irSensor;
Motordriver motors;
MotorController motorController(motors, DEFAULT_BASE_SPEED, MAX_TURN, MAX_PWM);

PID pid(DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, IRSensor::CENTER_POSITION);

// WiFi-tunable parameters (passed by reference)
int   baseSpeedValue = DEFAULT_BASE_SPEED;
float turnGain       = 1.0f;

WifiPid wifi(pid, irSensor, baseSpeedValue, turnGain, DEFAULT_KP, DEFAULT_KI, DEFAULT_KD);

// ============ Setup ============
void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
    delay(200);

    if (DEBUG_SERIAL) Serial.println("Booting...");

    wifi.begin();
    if (DEBUG_SERIAL) Serial.println("WiFi started");

    if (DEBUG_SERIAL) Serial.println("Starting calibration...");
    irSensor.calibrate(2000);
    if (DEBUG_SERIAL) Serial.println("Calibration done");
}

// ============ Debug Output ============
void printDebugInfo(uint16_t position, int error, float pidOutput, int motorSpeed) {
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug < 300) return;

    Serial.print("Pos: ");   Serial.print(position);
    Serial.print(" | Err: "); Serial.print(error);
    Serial.print(" | PID: "); Serial.print(pidOutput, 3);
    Serial.print(" | Gain: "); Serial.print(turnGain, 2);
    Serial.print(" | MS: ");  Serial.print(motorSpeed);
    Serial.print(" | Base: "); Serial.print(baseSpeedValue);
    Serial.println();

    lastDebug = millis();
}

// ============ Main Loop ============
void loop() {
    wifi.handle();

    // Stop if not running
    if (!wifi.isRunning()) {
        motorController.stop();
        digitalWrite(LED_PIN, LOW);
        return;
    }

    digitalWrite(LED_PIN, HIGH);

    // Update motor controller base speed from WiFi
    motorController.setBaseSpeed(baseSpeedValue);

    // Read sensor position
    uint16_t position = irSensor.readPosition();

    // Compute PID output and apply turn gain
    float pidOutput = pid.compute(position);
    int motorSpeed = static_cast<int>(lroundf(pidOutput * turnGain));

    // Drive motors
    motorController.drive(motorSpeed);

    // Debug output
    if (DEBUG_PID) {
        int error = static_cast<int>(position) - static_cast<int>(IRSensor::CENTER_POSITION);
        printDebugInfo(position, error, pidOutput, motorSpeed);
    }
}
