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
    int setpoint;

public:
    PID(float p, float i, float d, int target);
    int compute(int current_value);
    void reset();
    void setTunings(float p, float i, float d);
};

#endif //LINEFOLLOWERCAR_PID_HPP