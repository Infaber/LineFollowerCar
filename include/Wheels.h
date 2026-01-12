#pragma once

#include <Arduino.h>
#include "Drive.hpp"

// Pin bundle for a single motor driver channel.
struct MotorPins {
    int pwm; // Enable/PWM pin
    int in1; // Direction pin 1
    int in2; // Direction pin 2
};

// Minimal motor wrapper.
// - No PID / line-follow logic here.
// - Speed convention: -255..255, sign = direction.
class Wheels {
  public:
    Wheels();

    // Call from setup() once.
    void begin();

    void setLeft(int speed);
    void setRight(int speed);

    void stop();

  private:
    Motordriver driver_;
};
