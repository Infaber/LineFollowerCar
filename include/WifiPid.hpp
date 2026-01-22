#ifndef LINEFOLLOWERCAR_WIFI_HPP
#define LINEFOLLOWERCAR_WIFI_HPP
#include <Arduino.h>

class PID;

class WifiPid {
public:
    WifiPid(PID& pidRef);

    void begin();
    void handle();

    float getKp() const;
    float getKi() const;
    float getKd() const;

private:
    PID& pid;

    float kp;
    float ki;
    float kd;

    void setupServer();
};

#endif //LINEFOLLOWERCAR_WIFI_HPP