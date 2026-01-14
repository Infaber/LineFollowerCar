#pragma once

#include <Arduino.h>

class Motordriver {
private:
    int AIN1_ = 32;
    int AIN2_ = 35;
    int PWMA_ = 34;

    int BIN1_ = 0;
    int BIN2_ = 2;
    int PWMB_ = 4;

public:
    Motordriver();
    void right_motor(int speed);
    void left_motor(int speed);
};
