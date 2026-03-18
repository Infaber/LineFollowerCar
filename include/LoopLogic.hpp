// LoopLogic.hpp – Extracted loop body for testability
#pragma once

#include "MotorController.hpp"
#include "PID.hpp"

// Interface for wifi state (allows faking in tests)
class IWifiState {
public:
    virtual ~IWifiState() {}
    virtual bool isRunning() const = 0;
    virtual void handle() = 0;
};

// Interface for sensor reading (allows faking in tests)
class ISensorReader {
public:
    virtual ~ISensorReader() {}
    virtual uint16_t readPosition() = 0;
};

// The extracted loop body – same logic as main.cpp loop()
inline void loopStep(IWifiState& wifi,
                     MotorController& motorController,
                     ISensorReader& sensor,
                     PID& pid,
                     int baseSpeedValue,
                     float turnGain,
                     uint8_t ledPin)
{
    wifi.handle();

    if (!wifi.isRunning()) {
        motorController.stop();
        digitalWrite(ledPin, LOW);
        return;
    }

    digitalWrite(ledPin, HIGH);
    motorController.setBaseSpeed(baseSpeedValue);

    uint16_t position = sensor.readPosition();

    float pidOutput = pid.compute(position);
    int motorSpeed = static_cast<int>(lroundf(pidOutput * turnGain));

    motorController.drive(motorSpeed);
}


