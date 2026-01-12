#include "Wheels.h"

Wheels::Wheels() = default;

void Wheels::begin() {
    // Motordriver's constructor configures pin modes.
    // This method exists to make setup() code explicit.
}

void Wheels::setLeft(int speed) {
    driver_.left_motor(speed);
}

void Wheels::setRight(int speed) {
    driver_.right_motor(speed);
}

void Wheels::stop() {
    driver_.left_motor(0);
    driver_.right_motor(0);
}

