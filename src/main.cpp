#include <Arduino.h>
#include <QTRSensors.h>
#include "Wheels.h"

#define LED 2  // Built-in LED pin for most ESP32 boards

void rightMotor(int speed);
void leftMotor(int speed);

// Wheels wrapper instance (hardware-only; PID stays below).
static Wheels wheels;

void rightMotor(int speed) {
    wheels.setRight(speed);
}

void leftMotor(int speed) {
    wheels.setLeft(speed);
}

QTRSensors qtr;

const uint8_t SensorCount = 15;
uint16_t sensorValues[SensorCount];

// Motor control pins



int lasterror = 0;

// PID
float kp = 0.065; // 0.08 //0.08
float ki = 0.00; //0.001 //0.0035 //0.0065
float kd = 0.22; //0.2 //0.15 //0.18
float I = 0;

int baseSpeedValue = 255; //255 på mange svinger bane, 200 på bane med 90 grader (sniffere mere)
uint16_t positionLine = 0;

uint16_t filteredPos[5] = {7000, 7000, 7000, 7000, 7000};

void update_list(uint16_t newValue, uint16_t filterlist[5]) {
    for (uint16_t i = 4; i > 0; i--) {
        filterlist[i] = filterlist[i - 1];
    }
    filterlist[0] = newValue;
}

uint16_t returnAvgFive(uint16_t filterlist[5]){
  uint16_t sum = 0;
  for(uint16_t i=0; i<5; i++){
    sum += filterlist[i];
  }
  return sum/5;
}


void setup()
{
  Serial.begin(115200);
  Serial.println("QTR calibration  (digital)");

  wheels.begin();

  // Use analog mode
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){33, 25, 26, 27, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23}, SensorCount);
  // remove setEmitterPin(2); if emitters tied to VCC

  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, HIGH);

  delay(500);

  for (uint16_t i = 0; i < 400; i++)
  {
    qtr.calibrate();
    delay(10);
  }
 // digitalWrite(LED_BUILTIN, LOW);

  // Motor pins


  Serial.println("Calibration is done");
}

void PID_controller() {


positionLine=0;
  for (uint8_t i=0; i<5; i++) {
    positionLine += qtr.readLineBlack(sensorValues);
  }
  positionLine /= 5;


  int error = 7000 - positionLine;

  int P = error;
  I = constrain(error + I, -100000, 100000); //-2000 & 2000
  int D = error - lasterror;
  lasterror = error;

  int motorSpeed = P * kp + I * ki + D * kd;
  int motorSpeedA = baseSpeedValue + motorSpeed;
  int motorSpeedB = baseSpeedValue - motorSpeed;

  rightMotor (constrain(motorSpeedA, 0, 255));
  leftMotor (constrain(motorSpeedB, 0, 255));
}

void loop()
{
  PID_controller();

  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }
    Serial.print("Position: ");
    Serial.println(positionLine);
    delay(100);
}