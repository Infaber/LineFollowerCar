#include "MotorController.hpp"

MotorController::MotorController(Motordriver& motors, int baseSpeed, int maxTurn, int maxPwm)
    : motors_(motors)
    , baseSpeed_(baseSpeed)
    , maxTurn_(maxTurn)
    , maxPwm_(maxPwm)
{
}

void MotorController::setBaseSpeed(int speed) {
    baseSpeed_ = speed;
}

int MotorController::getBaseSpeed() const {
    return baseSpeed_;
}

void MotorController::drive(int motorSpeed) {
    // Constrain turn amount to prevent oscillation
    motorSpeed = constrain(motorSpeed, -maxTurn_, maxTurn_);

    // Motor mixing
    int left  = baseSpeed_ - motorSpeed;
    int right = baseSpeed_ + motorSpeed;

    // Constrain to valid PWM range (supports reverse)
    left  = constrain(left,  -maxPwm_, maxPwm_);
    right = constrain(right, -maxPwm_, maxPwm_);

    // Apply to motors
    motors_.left_motor(left);
    motors_.right_motor(right);
}

void MotorController::stop() {
    motors_.left_motor(0);
    motors_.right_motor(0);
}

