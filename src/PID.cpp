#include "PID.hpp"

PID::PID(float p, float i, float d, int target) {
    kp = p;
    ki = i;
    kd = d;
    setpoint = target;
    previous_error = 0;
    integral = 0;
}

int PID::compute(int current_value) {
    //Beregn error
    int error = setpoint - current_value;

    //P term
    int P = error;

    //I term (med anti-windup)
    integral = constrain(integral + error, -100000, 100000);
    int I = integral;

    //D term
    int D = error - previous_error;
    previous_error = error;

    //Beregn korreksjon
    int correction = P * kp + I * ki + D * kd;

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