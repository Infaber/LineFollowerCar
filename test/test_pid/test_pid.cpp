// test_pid.cpp – Unit tests for the PID class
#include <unity.h>
#include "PID.hpp"
#include <cmath>

// Access the fake millis counter from stubs
extern unsigned long _fake_millis;

// ---- Helper ----
static void resetMillis() { _fake_millis = 0; }

// ================================================================
// 1. Output ≈ 0 when current_value == setpoint
// ================================================================
void test_pid_output_zero_at_setpoint(void) {
    resetMillis();
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);   // P-only

    float out = pid.compute(4000.0f, 0.01f); // dt = 10 ms
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, out);
}

// ================================================================
// 2a. Positive error → positive output  (car is to the RIGHT of
//     centre means position > setpoint → error < 0 → output < 0,
//     but here we test "car LEFT of centre" = position < setpoint)
// ================================================================
void test_pid_positive_error_gives_positive_output(void) {
    resetMillis();
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    // Position 3000 => error = 4000 - 3000 = +1000  →  output > 0
    float out = pid.compute(3000.0f, 0.01f);
    TEST_ASSERT_GREATER_THAN(0.0f, out);
}

// ================================================================
// 2b. Negative error → negative output
// ================================================================
void test_pid_negative_error_gives_negative_output(void) {
    resetMillis();
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    // Position 5000 => error = 4000 - 5000 = -1000  →  output < 0
    float out = pid.compute(5000.0f, 0.01f);
    TEST_ASSERT_LESS_THAN(0.0f, out);
}

// ================================================================
// 3. Integral windup clamping – integral should not grow forever
// ================================================================
void test_pid_integral_windup_is_clamped(void) {
    resetMillis();
    // Ki = 1, others = 0 → output = Ki * integral
    PID pid(0.0f, 1.0f, 0.0f, 4000.0f);
    pid.setIntegralLimit(100.0f);

    // Feed a constant error of +1000 many times with dt = 1 s
    for (int i = 0; i < 500; i++) {
        pid.compute(3000.0f, 1.0f);  // error = +1000
    }

    // With Ki=1, output = integral; integral should be clamped to ±100
    float out = pid.compute(3000.0f, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 100.0f, out);
}

// ================================================================
// 4. reset() should clear internal state
// ================================================================
void test_pid_reset_clears_state(void) {
    resetMillis();
    PID pid(1.0f, 1.0f, 1.0f, 4000.0f);

    // Drive state far from zero
    for (int i = 0; i < 50; i++) {
        pid.compute(3000.0f, 0.01f);
    }

    pid.reset();

    // After reset, compute at setpoint should give ≈ 0
    // (D-term will be 0 because previous_error was reset to 0)
    float out = pid.compute(4000.0f, 0.01f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, out);
}

// ================================================================
// 5. D-term reacts to changing error
// ================================================================
void test_pid_derivative_responds_to_change(void) {
    resetMillis();
    // Kd only
    PID pid(0.0f, 0.0f, 1.0f, 4000.0f);

    // First call sets previous_error
    pid.compute(3000.0f, 0.01f);  // error = +1000

    // Second call: error changes from +1000 to +500, derivative = (500-1000)/0.01 = -50000
    float out = pid.compute(3500.0f, 0.01f);
    TEST_ASSERT_LESS_THAN(0.0f, out);  // derivative is negative
}

// ================================================================
// 6. setTunings updates coefficients live
// ================================================================
void test_pid_set_tunings(void) {
    resetMillis();
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    float out1 = pid.compute(3000.0f, 0.01f);  // Kp=1, error=1000 → 1000
    pid.reset();

    pid.setTunings(2.0f, 0.0f, 0.0f);
    float out2 = pid.compute(3000.0f, 0.01f);  // Kp=2, error=1000 → 2000

    TEST_ASSERT_FLOAT_WITHIN(0.1f, out1 * 2.0f, out2);
}

// ================================================================
// 7. Original millis-based compute still works
// ================================================================
void test_pid_millis_based_compute(void) {
    _fake_millis = 1000;
    PID pid(1.0f, 0.0f, 0.0f, 4000.0f);

    _fake_millis = 1010;  // 10 ms later
    float out = pid.compute(3000.0f);  // uses millis
    TEST_ASSERT_GREATER_THAN(0.0f, out);
}

// ========================== Runner ==========================
void setUp(void) {}
void tearDown(void) {}

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_pid_output_zero_at_setpoint);
    RUN_TEST(test_pid_positive_error_gives_positive_output);
    RUN_TEST(test_pid_negative_error_gives_negative_output);
    RUN_TEST(test_pid_integral_windup_is_clamped);
    RUN_TEST(test_pid_reset_clears_state);
    RUN_TEST(test_pid_derivative_responds_to_change);
    RUN_TEST(test_pid_set_tunings);
    RUN_TEST(test_pid_millis_based_compute);

    return UNITY_END();
}

