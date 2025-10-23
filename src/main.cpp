#include <Arduino.h>

#define LED 2  // Built-in LED pin for most ESP32 boards

void setup() {
    pinMode(LED, OUTPUT);
}

void loop() {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
}
