#pragma once
#include <Arduino.h>

struct MotorPins {
    int pwm;   // Enable/PWM pin
    int in1;   // Direction pin 1
    int in2;   // Direction pin 2
};

