#ifndef LINEFOLLOWERCAR_WIFI_HPP
#define LINEFOLLOWERCAR_WIFI_HPP

#include <Arduino.h>

class PID;
class IRSensor;

class WifiPid {
public:
    WifiPid(PID& pidRef, IRSensor& irRef, int& baseSpeedRef,
            float startKp, float startKi, float startKd);

    void begin();
    void handle();

    void start();
    void stop();
    bool isRunning() const;

    float getKp() const;
    float getKi() const;
    float getKd() const;
    int   getBaseSpeed() const;

private:
    PID& pid;
    IRSensor& ir;
    int& baseSpeed;

    float kp;
    float ki;
    float kd;

    bool running;

    String calibrationStatus = "NOT DONE";
    bool calibrating = false;                 // CHANGE: non-blocking calibration state
    unsigned long calibStartMs = 0;           // CHANGE: when calibration started
    unsigned long calibDurationMs = 2000;     // CHANGE: total calibration time


    void setupServer();
};

#endif // LINEFOLLOWERCAR_WIFI_HPP