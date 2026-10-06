// Tap Express — small debounced digital input. No library needed.
#pragma once
#include <Arduino.h>

class DebouncedInput {
 public:
  void begin(uint8_t pin, uint8_t mode, bool activeLow, uint16_t debounceMs) {
    pin_ = pin;
    activeLow_ = activeLow;
    debounceMs_ = debounceMs;
    pinMode(pin_, mode);
    stable_ = lastRaw_ = readRaw();
    lastChange_ = millis();
  }

  // Call every loop. Returns true on the loop where the debounced state changes.
  bool update(uint32_t now) {
    bool raw = readRaw();
    if (raw != lastRaw_) {
      lastRaw_ = raw;
      lastChange_ = now;
    }
    if (raw != stable_ && (uint32_t)(now - lastChange_) >= debounceMs_) {
      stable_ = raw;
      return true;
    }
    return false;
  }

  bool pressed() const { return stable_; }

 private:
  bool readRaw() const { return (digitalRead(pin_) == LOW) == activeLow_; }

  uint8_t pin_ = 0;
  bool activeLow_ = true;
  uint16_t debounceMs_ = 20;
  bool stable_ = false;
  bool lastRaw_ = false;
  uint32_t lastChange_ = 0;
};
