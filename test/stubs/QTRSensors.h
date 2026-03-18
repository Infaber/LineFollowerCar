// QTRSensors.h stub for native unit testing
#pragma once

#include <cstdint>

class QTRSensors {
public:
    void setTypeRC() {}
    void setSensorPins(const uint8_t*, uint8_t) {}
    void calibrate() {}
    uint16_t readLineBlack(uint16_t*) { return 4000; }
};

