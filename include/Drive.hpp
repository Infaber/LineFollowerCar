#pragma once

#include <Arduino.h>

class Motordriver {
private:
    int AIN1_ = 27;
    int AIN2_ = 26;
    int PWMA_ = 25;

    int BIN1_ = 18;
    int BIN2_ = 5;
    int PWMB_ = 13;

public:
    Motordriver();
    void right_motor(int speed);
    void left_motor(int speed);
};
