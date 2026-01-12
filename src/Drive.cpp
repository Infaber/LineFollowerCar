#include "Drive.hpp"

Motordriver::Motordriver() {
    pinMode(AIN1_, OUTPUT);
    pinMode(AIN2_, OUTPUT);
    pinMode(PWMA_, OUTPUT);
    pinMode(BIN1_, OUTPUT);
    pinMode(BIN2_, OUTPUT);
    pinMode(PWMB_, OUTPUT);
}

void Motordriver::right_motor(int speed) {
    if (speed > 0) {
        digitalWrite(AIN1_, HIGH);
        digitalWrite(AIN2_, LOW);
    } else if (speed < 0) {
        digitalWrite(AIN1_, LOW);
        digitalWrite(AIN2_, HIGH);
    } else {
        digitalWrite(AIN1_, LOW);
        digitalWrite(AIN2_, LOW);
    }
    analogWrite(PWMA_, abs(speed));
}

void Motordriver::left_motor(int speed) {
    if (speed > 0) {
        digitalWrite(BIN1_, HIGH);
        digitalWrite(BIN2_, LOW);
    } else if (speed < 0) {
        digitalWrite(BIN1_, LOW);
        digitalWrite(BIN2_, HIGH);
    } else {
        digitalWrite(BIN1_, LOW);
        digitalWrite(BIN2_, LOW);
    }
    analogWrite(PWMB_, abs(speed));
}