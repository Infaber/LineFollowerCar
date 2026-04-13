#pragma once

#include <Arduino.h>

class Motordriver {
private:
    int AIN1_ = 10;
    int AIN2_ = 11;
    int PWMA_ = 9;

    int BIN1_ = 30;
    int BIN2_ = 21;
    int PWMB_ = 15;

public:
    Motordriver();
    void right_motor(int speed);
    void left_motor(int speed);
};
