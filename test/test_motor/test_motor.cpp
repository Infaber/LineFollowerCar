// test_motor.cpp – Unit tests for MotorController
#include <unity.h>
#include "Drive.hpp"
#include "MotorController.hpp"
#include <cmath>

extern void (*unit_test_left_motor_hook)(int);
extern void (*unit_test_right_motor_hook)(int);

// ======== Fake Motordriver capture hooks ========
static int recorded_left  = -999;
static int recorded_right = -999;

static void recordLeft(int speed) { recorded_left = speed; }
static void recordRight(int speed) { recorded_right = speed; }

static void resetRecorded() {
    recorded_left  = -999;
    recorded_right = -999;
}

// 1. drive(0) → same speed on both wheels
void test_drive_zero_equal_speeds(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    mc.drive(0);
    TEST_ASSERT_EQUAL_INT(130, recorded_left);
    TEST_ASSERT_EQUAL_INT(130, recorded_right);
}

// 2. Positive correction → left faster, right slower
void test_drive_positive_correction(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    mc.drive(50);
    TEST_ASSERT_EQUAL_INT(180, recorded_left);
    TEST_ASSERT_EQUAL_INT(80,  recorded_right);
}

// 3. Negative correction → right faster, left slower
void test_drive_negative_correction(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    mc.drive(-50);
    TEST_ASSERT_EQUAL_INT(80,  recorded_left);
    TEST_ASSERT_EQUAL_INT(180, recorded_right);
}

// 4. Motor values constrained to maxPwm
void test_drive_constrain_max_pwm(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 200, 225, 255);
    mc.drive(200);
    TEST_ASSERT_LESS_OR_EQUAL(255, recorded_left);
    TEST_ASSERT_GREATER_OR_EQUAL(-255, recorded_right);
}

// 5. Motor values constrained to -maxPwm
void test_drive_constrain_min_pwm(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 0, 300, 255);
    mc.drive(-300);
    TEST_ASSERT_GREATER_OR_EQUAL(-255, recorded_left);
    TEST_ASSERT_LESS_OR_EQUAL(255, recorded_right);
}

// 6. motorSpeed clamped to maxTurn
void test_drive_constrain_max_turn(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 100, 50, 255);
    mc.drive(200);   // clamped to 50
    TEST_ASSERT_EQUAL_INT(150, recorded_left);
    TEST_ASSERT_EQUAL_INT(50,  recorded_right);
}

// 7. stop() sets both motors to 0
void test_stop_sets_both_zero(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 130, 225, 255);
    mc.drive(80);
    mc.stop();
    TEST_ASSERT_EQUAL_INT(0, recorded_left);
    TEST_ASSERT_EQUAL_INT(0, recorded_right);
}

// 8. setBaseSpeed changes base for drive
void test_set_base_speed(void) {
    resetRecorded();
    Motordriver m;
    MotorController mc(m, 100, 225, 255);
    mc.setBaseSpeed(200);
    TEST_ASSERT_EQUAL_INT(200, mc.getBaseSpeed());
    mc.drive(0);
    TEST_ASSERT_EQUAL_INT(200, recorded_left);
    TEST_ASSERT_EQUAL_INT(200, recorded_right);
}

void setUp(void) {
    unit_test_left_motor_hook = recordLeft;
    unit_test_right_motor_hook = recordRight;
    resetRecorded();
}

void tearDown(void) {
    unit_test_left_motor_hook = nullptr;
    unit_test_right_motor_hook = nullptr;
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_drive_zero_equal_speeds);
    RUN_TEST(test_drive_positive_correction);
    RUN_TEST(test_drive_negative_correction);
    RUN_TEST(test_drive_constrain_max_pwm);
    RUN_TEST(test_drive_constrain_min_pwm);
    RUN_TEST(test_drive_constrain_max_turn);
    RUN_TEST(test_stop_sets_both_zero);
    RUN_TEST(test_set_base_speed);
    return UNITY_END();
}
