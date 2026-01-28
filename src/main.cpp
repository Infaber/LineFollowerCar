#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "PID.hpp"
#include "WifiPid.hpp"

#define LED 2

// Debug switches (runtime)
const bool DEBUG_SERIAL  = true;
const bool DEBUG_PID     = true;
const bool DEBUG_LOOP_HZ = false;  // set true if you want loop Hz printed

IRSensor irSensor;
Motordriver motors;

// PID defaults
PID pid(0.03, 0.00, 0.10, IRSensor::CENTER_POSITION);

// Speed limits
int baseSpeedValue = 100;
const int MAX_MOTOR_SPEED = 150;

// WiFi PID tuner (initial values)
WifiPid wifi(pid, irSensor, baseSpeedValue, 0.03, 0.00, 0.10);

static void printLoopHz() {
    static unsigned long lastHzTime = 0;
    static uint32_t loopCounter = 0;
    loopCounter++;
    if (millis() - lastHzTime >= 1000) {
        Serial.print("Loop Hz: ");
        Serial.println(loopCounter);
        loopCounter = 0;
        lastHzTime = millis();
    }
}

void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    Serial.begin(115200);
    delay(200);

    if (DEBUG_SERIAL) Serial.println("Booting...");

    // If IRSensor has a begin(), call it (uncomment if your class provides it)
    // irSensor.begin();

    wifi.begin();
    if (DEBUG_SERIAL) Serial.println("WiFi started");

    // Optional: calibrate on boot (you can remove this and use the web button instead)
    if (DEBUG_SERIAL) Serial.println("Starting IR sensor calibration...");
    irSensor.calibrate(2000);
    if (DEBUG_SERIAL) Serial.println("Calibration done");

    // IMPORTANT: ensure the robot starts immediately (so you get prints + motion without pressing START)
    // Comment this out if you only want to start from the webpage.
    wifi.start();
}

void loop() {
    wifi.handle();  // keep webserver responsive

    if (DEBUG_LOOP_HZ) {
        printLoopHz();
    }

    // Always print debug values periodically (even when stopped)
    if (DEBUG_PID) {
        static unsigned long lastDebug = 0;
        if (millis() - lastDebug >= 200) {  // faster updates; change to 500 if you want less spam
            Serial.print("RUN: "); Serial.print(wifi.isRunning() ? "1" : "0");
            Serial.print(" | Base: "); Serial.print(baseSpeedValue);

            if (wifi.isRunning()) {
                uint16_t pos = irSensor.readPosition();
                Serial.print(" | Pos: "); Serial.print(pos);

                float corrF = pid.compute(pos);
                int correction = (int)roundf(corrF);

                int maxCorr = baseSpeedValue + 20;
                correction = constrain(correction, -maxCorr, maxCorr);

                int left  = baseSpeedValue - correction;
                int right = baseSpeedValue + correction;

                left  = constrain(left, 0, MAX_MOTOR_SPEED);
                right = constrain(right, 0, MAX_MOTOR_SPEED);

                Serial.print(" | Corr: "); Serial.print(correction);
                Serial.print(" | L: "); Serial.print(left);
                Serial.print(" | R: "); Serial.print(right);
            } else {
                Serial.print(" | Pos: --- | Corr: --- | L: 0 | R: 0");
            }

            Serial.println();
            lastDebug = millis();
        }
    }

    // If not running, keep motors stopped and exit early
    if (!wifi.isRunning()) {
        motors.left_motor(0);
        motors.right_motor(0);
        digitalWrite(LED, LOW);
        return;
    }

    digitalWrite(LED, HIGH);

    // Normal line following (run every loop)
    uint16_t position = irSensor.readPosition();

    float corrF = pid.compute(position);
    int correction = (int)roundf(corrF);

    int maxCorr = baseSpeedValue + 20;
    correction = constrain(correction, -maxCorr, maxCorr);

    int left  = baseSpeedValue - correction;
    int right = baseSpeedValue + correction;

    left  = constrain(left, 0, MAX_MOTOR_SPEED);
    right = constrain(right, 0, MAX_MOTOR_SPEED);

    motors.left_motor(left);
    motors.right_motor(right);
}