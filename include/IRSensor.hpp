#pragma once
#include <Arduino.h>
#include <QTRSensors.h>

class IRSensor {
public:
    static const uint8_t SENSOR_COUNT = 8;
    static const uint8_t FILTER_SIZE = 5;
    static const uint16_t CENTER_POSITION = 7000;  // For 8 sensorer (0-14000 skala)

    IRSensor();
    ~IRSensor();

    // Kalibrering
    void calibrate(uint16_t duration_ms = 4000);
    bool isCalibrated();

    // Lesing
    uint16_t readPosition();
    uint16_t getFilteredPosition();
    void getSensorValues(uint16_t* values);
    uint16_t getRawPosition();

    // Debug
    void printSensorValues();

private:
    QTRSensors qtr;
    uint16_t sensorValues[SENSOR_COUNT];
    uint16_t filteredPos[FILTER_SIZE];
    bool calibrated;
    uint16_t currentPosition;

    // Private hjelpefunksjoner
    void updateFilter(uint16_t newValue);
    uint16_t getFilterAverage();
};