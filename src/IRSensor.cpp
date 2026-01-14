#include "IRSensor.hpp"

IRSensor::IRSensor() {
    // Initialize QTR sensor
    qtr.setTypeRC();
    qtr.setSensorPins((const uint8_t[]){33, 25, 26, 27, 12, 13, 14, 15}, SENSOR_COUNT);

    // Initialize sensor values array
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        sensorValues[i] = 0;
    }

    // Initialize filter array with center position
    for (uint8_t i = 0; i < FILTER_SIZE; i++) {
        filteredPos[i] = CENTER_POSITION;
    }

    currentPosition = CENTER_POSITION;
    calibrated = false;
}

IRSensor::~IRSensor() {
    // Destructor - cleanup if needed
}

void IRSensor::calibrate(uint16_t duration_ms) {
    Serial.println("Starting IR sensor calibration...");

    uint16_t calibrationCycles = duration_ms / 10;

    for (uint16_t i = 0; i < calibrationCycles; i++) {
        qtr.calibrate();
        delay(10);
    }

    calibrated = true;
    Serial.println("IR sensor calibration complete!");
}

uint16_t IRSensor::readPosition() {
    if (!calibrated) {
        Serial.println("Warning: Sensor not calibrated!");
        return CENTER_POSITION;
    }

    // Read line position (average of 5 readings for stability)
    uint16_t position = 0;
    for (uint8_t i = 0; i < 5; i++) {
        position += qtr.readLineBlack(sensorValues);
    }
    position /= 5;

    currentPosition = position;

    // Update filter
    updateFilter(position);

    return position;
}

uint16_t IRSensor::getRawPosition() {
    return currentPosition;
}

uint16_t IRSensor::getFilteredPosition() {
    return getFilterAverage();
}

void IRSensor::getSensorValues(uint16_t* values) {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        values[i] = sensorValues[i];
    }
}

void IRSensor::printSensorValues() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        Serial.print(sensorValues[i]);
        Serial.print('\t');
    }
}

bool IRSensor::isCalibrated() {
    return calibrated;
}

void IRSensor::updateFilter(uint16_t newValue) {
    // Shift values in filter array
    for (uint8_t i = FILTER_SIZE - 1; i > 0; i--) {
        filteredPos[i] = filteredPos[i - 1];
    }
    filteredPos[0] = newValue;
}

uint16_t IRSensor::getFilterAverage() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < FILTER_SIZE; i++) {
        sum += filteredPos[i];
    }
    return sum / FILTER_SIZE;
}