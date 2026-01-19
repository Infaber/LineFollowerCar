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
int baseSpeedValue = 150; // 255 på mange svinger bane, 200 på bane med 90 grader

void setup() {
  Serial.begin(115200);
  Serial.println("Line follower robot starting...");

  delay(500);

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
  int motorSpeedA = baseSpeedValue + correction;
  int motorSpeedB = baseSpeedValue - correction;

  // Sett motor hastigheter (begrens til 0-255)
  motors.right_motor(constrain(motorSpeedA, 0, 150));
  motors.left_motor(constrain(motorSpeedB, 0, 150));

  // Debug output
  irSensor.printSensorValues();
  Serial.print("Position: ");
  Serial.print(position);
  Serial.print("\tCorrection: ");
  Serial.println(correction);

  delay(500);
}