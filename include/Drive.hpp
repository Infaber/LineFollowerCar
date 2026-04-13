#pragma once

#include <Arduino.h>

class Motordriver {
private:
    int AIN1_ = 27;
    int AIN2_ = 14;
    int PWMA_ = 26;

    int BIN1_ = 12;
    int BIN2_ = 21;
    int PWMB_ = 13;

public:
    Motordriver();
    void right_motor(int speed);
    void left_motor(int speed);
};