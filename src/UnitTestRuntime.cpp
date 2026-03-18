#if defined(UNIT_TEST) || defined(PIO_UNIT_TESTING)
#include "Arduino.h"
#include "Drive.hpp"

unsigned long _fake_millis = 0;
FakeSerial Serial;

void (*unit_test_left_motor_hook)(int) = nullptr;
void (*unit_test_right_motor_hook)(int) = nullptr;

Motordriver::Motordriver() {}
void Motordriver::left_motor(int speed) {
    if (unit_test_left_motor_hook) {
        unit_test_left_motor_hook(speed);
    }
}
void Motordriver::right_motor(int speed) {
    if (unit_test_right_motor_hook) {
        unit_test_right_motor_hook(speed);
    }
}
#endif
