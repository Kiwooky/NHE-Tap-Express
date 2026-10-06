// Minimal host-side Arduino stub so the firmware logic can be unit-tested on a PC.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define A0 14
#define A1 15
#define TWO_PI 6.283185307179586476925286766559
#define F(s) (reinterpret_cast<const __FlashStringHelper*>(s))
class __FlashStringHelper;
extern int simPin[32];      // digital levels the sketch reads / writes
extern int simPwm[32];      // last analogWrite value
extern int simAnalog[32];
extern uint32_t simMicros;
inline void pinMode(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t p) { return simPin[p]; }
inline void digitalWrite(uint8_t p, int v) { simPin[p] = v ? 1 : 0; }
inline void analogWrite(uint8_t p, int v) { simPwm[p] = v; }
inline int analogRead(uint8_t p) { return simAnalog[p]; }
inline uint32_t millis() { return simMicros / 1000; }
inline uint32_t micros() { return simMicros; }
inline long map(long x, long a, long b, long c, long d) { return (x - a) * (d - c) / (b - a) + c; }
inline long random(long lo, long hi) { return lo + rand() % (hi - lo); }
inline void randomSeed(unsigned long s) { srand(s); }
