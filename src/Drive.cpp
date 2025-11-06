#include "Drive.hpp"

Motordriver(){
    pinMode(AIN1_, OUTPUT);
    pinMode(AIN2_, OUTPUT);
    pinMode(PWMA_, OUTPUT);
    pinMode(BIN1_, OUTPUT);
    pinMode(BIN2_, OUTPUT);
    pinMode(PWMB_, OUTPUT);
}

void Motordriver::right_motor(int speed){
    if (speed > 0) {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    } else if (speed < 0) {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
    } else {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
    }
    analogWrite(PWMA, abs(speed));
}

void Motordriver::left_motor(int speed){
    if (speed > 0) {
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
    } else if (speed < 0) {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
    } else {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
    }
    analogWrite(PWMB, abs(speed));
}