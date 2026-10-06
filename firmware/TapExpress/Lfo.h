// Tap Express — waveform generator. Phase runs 0..1 per cycle, output is 0..1.
// Replaces the JLed objects from v2.3: one phase accumulator, so tempo changes
// are glitch-free and every LED stays in sync.
#pragma once
#include <Arduino.h>

enum Wave : uint8_t {
  WAVE_OFF,
  WAVE_ON,        // constant — the ramp / swell mode
  WAVE_SINE,
  WAVE_TRIANGLE,
  WAVE_SAW_UP,
  WAVE_SAW_DOWN,
  WAVE_SQUARE,
  WAVE_RANDOM,
};

class Lfo {
 public:
  void reset() {
    phase_ = 0.0f;
    lastSegment_ = 255;
  }

  // Advance by dtMs for a cycle of periodMs. Returns true when the cycle wraps.
  bool advance(float dtMs, float periodMs) {
    if (periodMs < 1.0f) periodMs = 1.0f;
    phase_ += dtMs / periodMs;
    if (phase_ >= 1.0f) {
      phase_ -= (float)(int)phase_;
      return true;
    }
    return false;
  }

  float phase() const { return phase_; }

  float value(Wave w, uint8_t randomSteps) {
    const float p = phase_;
    switch (w) {
      case WAVE_ON:       return 1.0f;
      case WAVE_SINE:     return 0.5f - 0.5f * cos(TWO_PI * p);   // starts at 0, like JLed Breathe
      case WAVE_TRIANGLE: return p < 0.5f ? 2.0f * p : 2.0f - 2.0f * p;
      case WAVE_SAW_UP:   return p;
      case WAVE_SAW_DOWN: return 1.0f - p;
      case WAVE_SQUARE:   return p < 0.5f ? 1.0f : 0.0f;          // starts high, like JLed Blink
      case WAVE_RANDOM:   return randomValue(p, randomSteps);
      case WAVE_OFF:
      default:            return 0.0f;
    }
  }

 private:
  // Smoothed sample-and-hold: pick a new target every 1/steps of a cycle and
  // ease towards it.
  float randomValue(float p, uint8_t steps) {
    if (steps == 0) steps = 1;
    uint8_t seg = (uint8_t)(p * steps);
    if (seg >= steps) seg = steps - 1;
    if (seg != lastSegment_) {
      lastSegment_ = seg;
      from_ = to_;
      to_ = random(0, 1001) / 1000.0f;
    }
    float t = p * steps - seg;
    t = t * t * (3.0f - 2.0f * t);  // smoothstep
    return from_ + (to_ - from_) * t;
  }

  float phase_ = 0.0f;
  uint8_t lastSegment_ = 255;
  float from_ = 0.0f;
  float to_ = 0.0f;
};
