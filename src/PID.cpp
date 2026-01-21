#include "PID.hpp"

PID::PID(float p, float i, float d, float target) {
    kp = p;
    ki = i;
    kd = d;
    setpoint = target;
    previous_error = 0;
    integral = 0;
}

float PID::compute(float current_value) {
    //Beregn error
    float error = setpoint - current_value;

    //P term
    float P = error;

    //I term (med anti-windup)
    integral = constrain(integral + error, -100000, 100000);
    float I = integral;

    //D term
    float D = error - previous_error;
    previous_error = error;

    //Beregn korreksjon
    float correction = P * kp + I * ki + D * kd;

    return correction;
}

void PID::reset() {
    previous_error = 0;
    integral = 0;
}

void PID::setTunings(float p, float i, float d) {
    kp = p;
    ki = i;
    kd = d;
}