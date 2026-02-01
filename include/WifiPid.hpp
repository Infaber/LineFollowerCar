#ifndef LINEFOLLOWERCAR_WIFI_HPP
#define LINEFOLLOWERCAR_WIFI_HPP

#include <Arduino.h>

class PID;
class IRSensor;

class WifiPid {
public:
    WifiPid(PID& pidRef, IRSensor& irRef, int& baseSpeedRef, float& turnGainRef,
            float startKp, float startKi, float startKd);

    void begin();
    void handle();

    void start();
    void stopp();
    bool isRunning() const;

    float getKp() const;
    float getKi() const;
    float getKd() const;

    int   getBaseSpeed() const;
    float getTurnGain() const;

private:
    PID& pid;
    IRSensor& ir;
    int& baseSpeed;
    float& turnGain;

    float kp;
    float ki;
    float kd;

    bool running;

    String calibrationStatus = "IKKE KALIBRERT";

    void setupServer();
};

#endif // LINEFOLLOWERCAR_WIFI_HPP
