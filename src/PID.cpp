// PID.cpp
#include "PID.hpp"

PID::PID(float p, float i, float d, float target) {
    kp = p;
    ki = i;
    kd = d;

    setpoint = target;

    previous_error = 0.0f;
    integral = 0.0f;

    integral_limit = 300.0f;      // default, tune this
    lastTime = millis();          // initialize timer
}

void PID::setIntegralLimit(float limit) {
    // Safety: don’t allow negative limits
    if (limit < 0) limit = -limit;
    integral_limit = limit;
}

void PID::setSetpoint(float target) {
    setpoint = target;
}

float PID::compute(float current_value) {
    // dt in seconds
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    // Protect against divide-by-zero / very small dt
    if (dt <= 0.0f) dt = 0.001f;

    // Error
    float error = setpoint - current_value;

    // P term
    float P = error;

    // I term (integrate with dt, clamp to prevent windup)
    integral = constrain(integral + error * dt, -integral_limit, integral_limit);
    float I = integral;

    // D term (derivative of error)
    float D = (error - previous_error) / dt;
    previous_error = error;

    // PID output
    float correction = (kp * P) + (ki * I) + (kd * D);
    return correction;
}

void PID::reset() {
    previous_error = 0.0f;
    integral = 0.0f;
    lastTime = millis(); // avoid huge dt after reset
}

void PID::setTunings(float p, float i, float d) {
    kp = p;
    ki = i;
    kd = d;
}
