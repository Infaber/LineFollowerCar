#ifndef LINEFOLLOWERCAR_WIFI_HPP
#define LINEFOLLOWERCAR_WIFI_HPP

#include <Arduino.h>

class PID;

class WifiPid {
public:
    WifiPid(PID& pidRef, float startKp, float startKi, float startKd);

    void begin();
    void handle();

    //Start/Stop kontroll
    void start();
    void stop();
    bool isRunning() const;

    //(Valgfritt) lese PID-verdier
    float getKp() const;
    float getKi() const;
    float getKd() const;

private:
    PID& pid;

    float kp;
    float ki;
    float kd;

    bool running;

    void setupServer();
};

#endif // LINEFOLLOWERCAR_WIFI_HPP