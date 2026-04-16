#pragma once

#include <Arduino.h>
#include "Drive.hpp"

class MotorController {
private:
    Motordriver& motors_;

    int baseSpeed_;
    int maxTurn_;
    int maxPwm_;

public:
    MotorController(Motordriver& motors, int baseSpeed = 130, int maxTurn = 225, int maxPwm = 255);

    // Set base speed
    void setBaseSpeed(int speed);
    int getBaseSpeed() const;

    // Set max turn
    void setMaxTurn(int maxTurn);
    int getMaxTurn() const;

    // Apply motor speeds based on PID output
    void drive(int motorSpeed);

    // Stop both motors
    void stop();
};