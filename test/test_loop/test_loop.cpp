// test_loop.cpp – Unit tests for main loop logic
//
// Tests the extracted loopStep() function with fake dependencies.
// Verifies:
//   - Car stops when wifi.isRunning() == false
//   - Car reads sensor, computes PID, drives motors when running

#include <unity.h>
#include "Drive.hpp"
#include "MotorController.hpp"
#include "LoopLogic.hpp"
#include <cmath>

extern unsigned long _fake_millis;
extern void (*unit_test_left_motor_hook)(int);
extern void (*unit_test_right_motor_hook)(int);

// ======== Fake Motordriver (records calls) ========
static int recorded_left  = -999;
static int recorded_right = -999;

static void recordLeft(int speed) { recorded_left = speed; }
static void recordRight(int speed) { recorded_right = speed; }

// ======== Fake Wifi ========
class FakeWifi : public IWifiState {
public:
    bool running = false;
    int  handleCount = 0;

    bool isRunning() const override { return running; }
    void handle() override { handleCount++; }
};

// ======== Fake Sensor ========
class FakeSensor : public ISensorReader {
public:
    uint16_t position = 4000;  // centre by default
    int readCount = 0;

    uint16_t readPosition() override {
        readCount++;
        return position;
    }
};

// ======== Helpers ========
static void resetAll() {
    recorded_left  = -999;
    recorded_right = -999;
    _fake_millis   = 0;
}

// ================================================================
// 1. When wifi.isRunning() == false → motors stop (both 0)
// ================================================================
void test_loop_stops_when_not_running(void) {
    resetAll();

    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    wifi.running = false;

    // Drive something first so we can verify stop resets it
    mc.drive(100);
    TEST_ASSERT_NOT_EQUAL(0, recorded_left);

    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);

    TEST_ASSERT_EQUAL_INT(0, recorded_left);
    TEST_ASSERT_EQUAL_INT(0, recorded_right);
}

// ================================================================
// 2. When wifi.isRunning() == false → sensor is NOT read
// ================================================================
void test_loop_does_not_read_sensor_when_stopped(void) {
    resetAll();

    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    wifi.running = false;
    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);

    TEST_ASSERT_EQUAL_INT(0, sensor.readCount);
}

// ================================================================
// 3. When wifi.isRunning() == true → sensor IS read
// ================================================================
void test_loop_reads_sensor_when_running(void) {
    resetAll();
    _fake_millis = 100;

    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    wifi.running = true;
    sensor.position = 3500;

    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);

    TEST_ASSERT_EQUAL_INT(1, sensor.readCount);
}

// ================================================================
// 4. When running with sensor at centre → drive(~0) → equal motors
// ================================================================
void test_loop_drives_straight_at_centre(void) {
    resetAll();
    _fake_millis = 100;

    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);  // P-only, setpoint=4000

    wifi.running = true;
    sensor.position = 4000;  // exactly at centre → error=0

    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);

    // PID output ≈ 0 → motorSpeed ≈ 0 → both wheels at baseSpeed
    TEST_ASSERT_EQUAL_INT(130, recorded_left);
    TEST_ASSERT_EQUAL_INT(130, recorded_right);
}

// ================================================================
// 5. When running with sensor off-centre → motors differ
// ================================================================
void test_loop_steers_when_off_centre(void) {
    resetAll();
    _fake_millis = 100;

    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    wifi.running = true;
    sensor.position = 3000;  // error = +1000 → positive correction

    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);

    // Left should be faster than right
    TEST_ASSERT_GREATER_THAN(recorded_right, recorded_left);
}

// ================================================================
// 6. wifi.handle() is always called (even when stopped)
// ================================================================
void test_loop_always_calls_wifi_handle(void) {
    resetAll();

    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    wifi.running = false;
    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);
    TEST_ASSERT_EQUAL_INT(1, wifi.handleCount);

    wifi.running = true;
    _fake_millis = 100;
    loopStep(wifi, mc, sensor, pid, 130, 1.0f, 2);
    TEST_ASSERT_EQUAL_INT(2, wifi.handleCount);
}

// ================================================================
// 7. baseSpeed parameter is forwarded to MotorController
// ================================================================
void test_loop_uses_basespeed_param(void) {
    resetAll();
    _fake_millis = 100;

    Motordriver m;
    MotorController mc(m, 100, 225, 255);
    FakeWifi wifi;
    FakeSensor sensor;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    wifi.running = true;
    sensor.position = 4000;

    loopStep(wifi, mc, sensor, pid, 200, 1.0f, 2);

    // baseSpeed updated to 200, sensor at centre → both at 200
    TEST_ASSERT_EQUAL_INT(200, recorded_left);
    TEST_ASSERT_EQUAL_INT(200, recorded_right);
}

// ========================== Runner ==========================
void setUp(void) {
    unit_test_left_motor_hook = recordLeft;
    unit_test_right_motor_hook = recordRight;
    resetAll();
}

void tearDown(void) {
    unit_test_left_motor_hook = nullptr;
    unit_test_right_motor_hook = nullptr;
}

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_loop_stops_when_not_running);
    RUN_TEST(test_loop_does_not_read_sensor_when_stopped);
    RUN_TEST(test_loop_reads_sensor_when_running);
    RUN_TEST(test_loop_drives_straight_at_centre);
    RUN_TEST(test_loop_steers_when_off_centre);
    RUN_TEST(test_loop_always_calls_wifi_handle);
    RUN_TEST(test_loop_uses_basespeed_param);

    return UNITY_END();
}

