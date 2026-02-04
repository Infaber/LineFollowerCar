// PID.hpp
#ifndef LINEFOLLOWERCAR_PID_HPP
#define LINEFOLLOWERCAR_PID_HPP

#include <Arduino.h>

class PID {
private:
    float kp;
    float ki;
    float kd;

    float previous_error;
    float integral;
    float setpoint;

    float integral_limit;       // integral clamp limit
    unsigned long lastTime;     // for dt calculation (ms)

public:
    PID(float p, float i, float d, float target);

    float compute(float current_value);

    void reset();

    void setTunings(float p, float i, float d);

    void setSetpoint(float target);

    void setIntegralLimit(float limit);
};

#endif // LINEFOLLOWERCAR_PID_HPP
