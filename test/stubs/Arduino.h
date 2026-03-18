// Arduino.h stub for native (PC) unit testing
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <string>

// --- Types ---
typedef uint8_t byte;
typedef bool boolean;

// --- Pin modes ---
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define HIGH 1
#define LOW  0

#define LED_BUILTIN 2

// --- Fake millis ---
// Controllable from tests
extern unsigned long _fake_millis;
inline unsigned long millis() { return _fake_millis; }
inline unsigned long micros() { return _fake_millis * 1000UL; }
inline void delay(unsigned long ms) { _fake_millis += ms; }
inline void delayMicroseconds(unsigned int us) { _fake_millis += us / 1000; }

// --- GPIO stubs ---
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline int  digitalRead(uint8_t) { return 0; }
inline void analogWrite(uint8_t, int) {}
inline int  analogRead(uint8_t) { return 0; }

// --- Math helpers ---
template <typename T>
inline T constrain(T val, T lo, T hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <typename T>
inline T _min(T a, T b) { return (a < b) ? a : b; }
template <typename T>
inline T _max(T a, T b) { return (a > b) ? a : b; }

#ifndef min
#define min(a,b) _min(a,b)
#endif
#ifndef max
#define max(a,b) _max(a,b)
#endif

// --- Minimal String class ---
class String {
    std::string s_;
public:
    String() {}
    String(const char* c) : s_(c ? c : "") {}
    String(const std::string& st) : s_(st) {}
    String(int v)   : s_(std::to_string(v)) {}
    String(float v, int dec = 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", dec, (double)v);
        s_ = buf;
    }
    String(unsigned int v) : s_(std::to_string(v)) {}
    String(long v) : s_(std::to_string(v)) {}
    String(unsigned long v) : s_(std::to_string(v)) {}
    String(bool b) : s_(b ? "true" : "false") {}

    const char* c_str() const { return s_.c_str(); }
    int length() const { return (int)s_.size(); }
    float toFloat() const { return std::stof(s_); }
    int toInt() const { return std::stoi(s_); }

    String operator+(const String& o) const { return String(s_ + o.s_); }
    String& operator+=(const String& o) { s_ += o.s_; return *this; }
    bool operator==(const String& o) const { return s_ == o.s_; }
    bool operator!=(const String& o) const { return s_ != o.s_; }
};

// --- Serial stub ---
class FakeSerial {
public:
    void begin(unsigned long) {}
    void print(const char*) {}
    void print(int) {}
    void print(float, int = 2) {}
    void print(unsigned int) {}
    void print(const String&) {}
    void println() {}
    void println(const char*) {}
    void println(int) {}
    void println(float, int = 2) {}
    void println(const String&) {}
    int printf(const char*, ...) { return 0; }
};

extern FakeSerial Serial;

// --- WIFI_AP stub (used in WifiPid) ---
#define WIFI_AP 2

