#include <Arduino.h>
#include "IRSensor.hpp"
#include "Drive.hpp"
#include "PID.hpp"

#define LED 2  // Built-in LED pin for most ESP32 boards

//Objekt instanser
IRSensor irSensor;
Motordriver motors;
PID pid(0.065, 0.00, 0.22, IRSensor::CENTER_POSITION);

//Kjøre parametere
int baseSpeedValue = 100; // 255 på mange svinger bane, 200 på bane med 90 grader

void setup() {
    Serial.begin(115200);
    Serial.println("Line follower robot starting...");

    // TEST HØYRE MOTOR DIREKTE
    Serial.println("Testing RIGHT motor at 150 speed...");
    motors.right_motor(150);
    delay(3000);
    motors.right_motor(0);

    Serial.println("Testing LEFT motor at 150 speed...");
    motors.left_motor(150);
    delay(3000);
    motors.left_motor(0);

    // Kalibrer IR sensoren (4 sekunder)
    irSensor.calibrate(2000);

    Serial.println("Setup complete! Starting line following...");
}

void loop() {
  //Les posisjon fra IR sensor
  uint16_t position = irSensor.readPosition();

  //Beregn PID korreksjon
  int correction = pid.compute(position);

  //Kalkuler motor hastigheter
  int motorSpeedA = baseSpeedValue - correction;
  int motorSpeedB = baseSpeedValue + correction;

  // Sett motor hastigheter (begrens til 0-255)
  motors.right_motor((motorSpeedA, 0, 125));
  motors.left_motor((motorSpeedB, 0, 125));



  // Debug output
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
        Serial.print("Pos: "); Serial.print(position);
        Serial.print(" | Corr: "); Serial.print(correction);
        Serial.print(" | L: "); Serial.print(motorSpeedB);
        Serial.print(" | R: "); Serial.println(motorSpeedA);
        lastDebug = millis();
    }
}