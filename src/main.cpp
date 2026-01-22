#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "PID.hpp"
#include "WifiPid.hpp"

#define LED 2

IRSensor irSensor;
Motordriver motors;
PID pid(0.03, 0.00, 0.10, IRSensor::CENTER_POSITION);
WifiPid wifi(pid);

// Kjøre parametere
int baseSpeedValue = 100;  // 255 på mange svinger bane, 200 på bane med 90 grader

void setup() {
    Serial.begin(115200);
    delay(200);

    wifi.begin();              // Start WiFi AP + webserver
    irSensor.calibrate(2000);  // Kalibrer sensorer
}

void loop() {
    wifi.handle(); // IMPORTANT: keeps the webserver alive!

    //Les posisjon fra IR sensor
    uint16_t position = irSensor.readPosition();

    //Beregn PID korreksjon
    int correction = (int)pid.compute(position);

    //(Valgfritt men anbefalt) begrens korreksjonen så den ikke klipper altfor hardt
    correction = constrain(correction, -80, 80);

    // Kalkuler motor hastigheter
    int left  = baseSpeedValue - correction;
    int right = baseSpeedValue + correction;

    // Begrens og sett motorer
    left  = constrain(left, 0, 150);
    right = constrain(right, 0, 150);

    motors.left_motor(left);
    motors.right_motor(right);

    // Debug output
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
        Serial.print("Pos: "); Serial.print(position);
        Serial.print(" | Corr: "); Serial.print(correction);
        Serial.print(" | L: "); Serial.print(left);
        Serial.print(" | R: "); Serial.println(right);
        lastDebug = millis();
    }
}
