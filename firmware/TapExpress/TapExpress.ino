/* =============================================================================
   Tap Express v4.0 — tap-tempo LFO + ramp for expression-pedal inputs

   An Arduino drives the LED of an LED/LDR (vactrol). The LDR sits on the
   expression input of a multi-effect (built for a Line6 M5/M9) and stands in
   for the pedal's potentiometer. Moving the LED brightness = moving the pedal.
   The same signal can also go out as a MIDI CC (0–127), or instead of the LDR.

   Controls (v1.1 board):
     Footswitch       tap = latch on/off, hold = momentary, double-tap = shift
     Waveform rotary  sine, ramp-up saw, square, ramp-down saw, random,
                      constant ON (ramp/swell), triangle in the spare slots
     Tempo pot        60–2000 ms per cycle (pot takes over when moved)
     Tap tempo        from the Line6 tap switch
     Multiply toggle  2x speed / tapped tempo / quarter speed (cycle = 0.5, 1 or 4 taps)
     Ramp toggle      ramp length in beats (2 / 4 / 8 by default)
     MIDI out (A1)    optional: the output as a CC, see config.h

   Ramp (constant ON mode):
     Hold   — ramps up while held. Release mid-ramp and it reverses back to off.
              At the top it stays on only while held; release ramps down and a
              press during that ramp-down reverses it again.
     Tap    — ramps up by itself and latches on at the top. Tap again to ramp
              down. A press at any point mid-ramp reverses direction.

   Everything is non-blocking: tempo, tap and LEDs keep running during a ramp.
   No external libraries required. All settings live in config.h.

   Original concept and v1–v2.3 by Niels, 2021. v4 rewrite 2026.
   ============================================================================= */

#include "Lfo.h"
#include "Input.h"
#include "MidiOut.h"
#include "config.h"

// ---------------------------------------------------------------------------
//  Lookup tables built from config.h
// ---------------------------------------------------------------------------
static const Wave WAVE_TABLE[8] = {
  WAVE_FOR_0, WAVE_FOR_1, WAVE_FOR_2, WAVE_FOR_3,
  WAVE_FOR_4, WAVE_FOR_5, WAVE_FOR_6, WAVE_FOR_7,
};
static const float MULT_TABLE[4] = { MULT_FOR_0, MULT_FOR_1, MULT_FOR_2, MULT_FOR_3 };
static const float RAMP_BEATS_TABLE[4] = {
  RAMP_BEATS_FOR_0, RAMP_BEATS_FOR_1, RAMP_BEATS_FOR_2, RAMP_BEATS_FOR_3,
};

static uint8_t outputCurve[256];  // 0..255 level -> PWM, with gamma and min/max applied

// ---------------------------------------------------------------------------
//  State
// ---------------------------------------------------------------------------
enum EngageState : uint8_t {
  IDLE,          // effect off, waiting for a press
  PENDING,       // pressed from idle — is it a click or a hold?
  WAIT_SECOND,   // released quickly — waiting to see if it's a double-click
  WAIT_RELEASE,  // swallow the rest of a press (after double-click / in shift)
  MOMENTARY,     // engaged by holding; follows the switch
  LATCHED,       // engaged by a click; each press toggles direction
};

static DebouncedInput footswitch;
static DebouncedInput tapInput;
static Lfo lfo;

static EngageState state = IDLE;
static uint32_t stateSince = 0;
static bool target = false;   // where the ramp is heading: true = on
static float level = 0.0f;    // 0..1 — ramp position / LFO depth

static bool shiftOn = false;
static bool shiftLinesActive = false;
static uint32_t shiftPulseUntil = 0;

static uint32_t basePeriodMs = 500;   // one beat
static bool potControls = true;
static int potSmoothed = 0;           // 0..1023
static int potAtTap = 0;
static uint32_t lastPotRead = 0;
static uint32_t lastTapMs = 0;
static bool haveTapped = false;
static uint32_t lastTapInterval = 0;  // 0 = no valid previous interval

static uint32_t lastMicros = 0;

#if OUTPUT_MIDI
static MidiOut midi;
static int32_t lastMidiValue = -1;    // -1 = nothing sent yet
static uint32_t lastMidiMs = 0;
#endif

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
static uint8_t readBits2(uint8_t a, uint8_t b) {
  return (digitalRead(a) << 1) | digitalRead(b);
}

static Wave currentWave() {
  uint8_t v = (digitalRead(PIN_WAVE_SEL_1) << 2) | (digitalRead(PIN_WAVE_SEL_2) << 1) |
              digitalRead(PIN_WAVE_SEL_3);
  return WAVE_TABLE[v];
}

static float multiplier() { return MULT_TABLE[readBits2(PIN_MULT_SEL_A, PIN_MULT_SEL_B)]; }

static float cyclePeriodMs() {
  float p = basePeriodMs * multiplier();
  return p < 20.0f ? 20.0f : p;
}

static float rampMs() {
  float r = basePeriodMs * RAMP_BEATS_TABLE[readBits2(PIN_RAMP_SEL_A, PIN_RAMP_SEL_B)];
  if (r < RAMP_MIN_MS) r = RAMP_MIN_MS;
  if (r > RAMP_MAX_MS) r = RAMP_MAX_MS;
  return r;
}

static bool waveRamps(Wave w) { return w == WAVE_ON || RAMP_LFO_WAVES; }

static void setState(EngageState s, uint32_t now) {
  state = s;
  stateSince = now;
}

static void debugPrint(const __FlashStringHelper* msg) {
#if DEBUG_SERIAL
  Serial.println(msg);
#else
  (void)msg;
#endif
}

static void buildOutputCurve() {
  for (int i = 0; i < 256; i++) {
    float x = i / 255.0f;
    float y = (OUTPUT_GAMMA == 1.0) ? x : pow(x, (float)OUTPUT_GAMMA);
    outputCurve[i] = (uint8_t)(OUTPUT_MIN + y * (OUTPUT_MAX - OUTPUT_MIN) + 0.5f);
  }
}

static void writeLdr(float x) {
#if OUTPUT_LDR
  uint8_t pwm = outputCurve[(uint8_t)(x * 255.0f + 0.5f)];
  if (INVERT_OUTPUT) pwm = 255 - pwm;
  analogWrite(PIN_OUTPUT, pwm);
#else
  (void)x;
#endif
}

// Sends the output as a CC, only when the value changes and no faster than
// MIDI_INTERVAL_MS. Linear: the gamma curve is only for the LDR.
static void writeMidi(float x, uint32_t now) {
#if OUTPUT_MIDI
  if (lastMidiValue >= 0 && now - lastMidiMs < MIDI_INTERVAL_MS) return;
  if (MIDI_INVERT) x = 1.0f - x;
  const float cc = MIDI_MIN + x * (MIDI_MAX - MIDI_MIN);   // 0..127 scale
#if MIDI_HIRES
  int32_t v = (int32_t)(cc * 128.0f + 0.5f);
  if (v > 16383) v = 16383;
  if (v == lastMidiValue) return;
  midi.controlChange(MIDI_CHANNEL, MIDI_CC, v >> 7);
  midi.controlChange(MIDI_CHANNEL, MIDI_CC + 32, v & 0x7F);
#else
  int32_t v = (int32_t)(cc + 0.5f);
  if (v == lastMidiValue) return;
  midi.controlChange(MIDI_CHANNEL, MIDI_CC, (uint8_t)v);
#endif
  lastMidiValue = v;
  lastMidiMs = now;
#else
  (void)x;
  (void)now;
#endif
}

static void writeOutput(float x, uint32_t now) {
  if (x < 0.0f) x = 0.0f;
  if (x > 1.0f) x = 1.0f;
  writeLdr(x);
  writeMidi(x, now);
}

// ---------------------------------------------------------------------------
//  Shift / Line6 preset select
// ---------------------------------------------------------------------------
static void setShiftLines(bool active) {
  shiftLinesActive = active;
  digitalWrite(PIN_L6_BANK_SELECT, active ? HIGH : LOW);   // CD4066 closes -> Line6 switches pressed
  digitalWrite(PIN_L6_TAP_DISABLE, active ? LOW : HIGH);   // CD4066 opens  -> tap line disconnected
}

static void toggleShift() {
  shiftOn = !shiftOn;
  digitalWrite(PIN_SHIFT_LED, shiftOn ? HIGH : LOW);
  if (!shiftOn) setShiftLines(false);   // never leave the Line6 switches held down
  debugPrint(shiftOn ? F("shift on") : F("shift off"));
}

static void shiftAction(uint32_t now) {
  if (SHIFT_PULSE_MS > 0) {
    setShiftLines(true);
    shiftPulseUntil = now + SHIFT_PULSE_MS;
  } else {
    setShiftLines(!shiftLinesActive);
  }
  debugPrint(F("preset select"));
}

// ---------------------------------------------------------------------------
//  Engage / footswitch state machine
// ---------------------------------------------------------------------------
static void engage(EngageState s, uint32_t now) {
  if (level <= 0.0f) lfo.reset();   // start the waveform from the top, like v2.3
  target = true;
  setState(s, now);
}

static void handleFootswitch(bool changed, uint32_t now) {
  const bool down = footswitch.pressed();
  const bool pressedEdge = changed && down;
  const bool releasedEdge = changed && !down;

  switch (state) {
    case IDLE:
      if (pressedEdge) setState(PENDING, now);
      break;

    case PENDING:
      if (releasedEdge) {
        setState(WAIT_SECOND, now);
      } else if (now - stateSince >= LONG_PRESS_MS) {
        if (shiftOn) {
          setState(WAIT_RELEASE, now);   // holds do nothing in shift mode
        } else {
          engage(MOMENTARY, now);
          debugPrint(F("momentary"));
        }
      }
      break;

    case WAIT_SECOND:
      if (pressedEdge) {
        toggleShift();                   // double-click
        setState(WAIT_RELEASE, now);
      } else if (now - stateSince >= DOUBLE_CLICK_MS) {
        if (shiftOn) {                   // single click
          shiftAction(now);
          setState(IDLE, now);
        } else {
          engage(LATCHED, now);
          debugPrint(F("latched"));
        }
      }
      break;

    case WAIT_RELEASE:
      if (!down) setState(IDLE, now);
      break;

    case MOMENTARY:
      if (changed) target = down;        // hold = up, release = down, re-press = up again
      break;

    case LATCHED:
      if (pressedEdge) target = !target; // any press reverses / switches off
      break;
  }
}

static void updateLevel(float dtMs, Wave w, uint32_t now) {
  if (state != MOMENTARY && state != LATCHED) {
    level = 0.0f;
    return;
  }
  if (!waveRamps(w)) {
    level = target ? 1.0f : 0.0f;
  } else {
    float step = dtMs / rampMs();
    level += target ? step : -step;
    if (level > 1.0f) level = 1.0f;
  }
  if (!target && level <= 0.0f) {
    level = 0.0f;
    setState(IDLE, now);                 // fully off — ready for the next press
    debugPrint(F("off"));
  }
}

// ---------------------------------------------------------------------------
//  Tempo
// ---------------------------------------------------------------------------
static uint32_t potToPeriod(int raw) {
  return map(raw, 0, 1023, POT_MIN_MS, POT_MAX_MS);
}

static void pollPot(uint32_t now) {
  if (now - lastPotRead < 5) return;
  lastPotRead = now;
  // Exponential moving average, 1/8 weight.
  potSmoothed += (analogRead(PIN_TEMPO_POT) - potSmoothed) / 8;

  if (!potControls && abs(potSmoothed - potAtTap) > POT_TAKEOVER) {
    potControls = true;
    debugPrint(F("pot takes over"));
  }
  if (potControls) basePeriodMs = potToPeriod(potSmoothed);
}

static void handleTap(uint32_t now) {
  uint32_t interval = now - lastTapMs;
  if (haveTapped && interval < TAP_MIN_MS) return;       // bounce
  lastTapMs = now;

  if (!haveTapped || interval > TAP_MAX_MS) {            // first tap of a new sequence
    haveTapped = true;
    lastTapInterval = 0;
  } else {
    basePeriodMs = lastTapInterval ? (interval + lastTapInterval) / 2 : interval;
    lastTapInterval = interval;
    potControls = false;
    potAtTap = potSmoothed;
    debugPrint(F("tap tempo"));
  }
  lfo.reset();                           // lock the phase to the tap
}

// ---------------------------------------------------------------------------
//  LEDs
// ---------------------------------------------------------------------------
static void updateIndicators(Wave w, float lfoValue, uint32_t now) {
  const bool engaged = (state == MOMENTARY || state == LATCHED);

  // Tempo flash: on for the first 5% of each cycle.
  bool flash = lfo.phase() < 0.05f;
  digitalWrite(PIN_TEMPO_LED, flash);
  if (PIN_MULT_LED >= 0) digitalWrite(PIN_MULT_LED, flash);

  // Preview: shows the selected waveform; in ON mode shows the ramp.
  float preview;
  if (w == WAVE_ON) preview = engaged ? level : PREVIEW_MAX / 255.0f;
  else              preview = lfoValue * PREVIEW_MAX / 255.0f;
  analogWrite(PIN_PREVIEW_LED, (uint8_t)(preview * 255.0f));

  // Ramp LED: only in ramp mode. Idle = slow breathe at the ramp speed.
  uint8_t rampLed = 0;
  if (waveRamps(w)) {
    if (engaged) {
      rampLed = (uint8_t)(level * 255.0f);
    } else {
      uint32_t r = (uint32_t)rampMs() * 2;
      float p = (now % r) / (float)r;
      rampLed = (uint8_t)((0.5f - 0.5f * cos(TWO_PI * p)) * PREVIEW_MAX);
    }
  }
  analogWrite(PIN_RAMP_LED, rampLed);
}

// ---------------------------------------------------------------------------
//  Arduino entry points
// ---------------------------------------------------------------------------
void setup() {
#if DEBUG_SERIAL
  Serial.begin(DEBUG_BAUD);
#endif
  buildOutputCurve();

#if OUTPUT_LDR
  pinMode(PIN_OUTPUT, OUTPUT);
#endif
#if OUTPUT_MIDI
  midi.begin(PIN_MIDI_OUT);
#endif
  writeOutput(0.0f, millis());

  const uint8_t selectors[] = {
    PIN_WAVE_SEL_1, PIN_WAVE_SEL_2, PIN_WAVE_SEL_3,
    PIN_MULT_SEL_A, PIN_MULT_SEL_B, PIN_RAMP_SEL_A, PIN_RAMP_SEL_B,
  };
  for (uint8_t pin : selectors) pinMode(pin, INPUT_PULLUP);

  const uint8_t leds[] = { PIN_PREVIEW_LED, PIN_RAMP_LED, PIN_TEMPO_LED, PIN_SHIFT_LED };
  for (uint8_t pin : leds) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  if (PIN_MULT_LED >= 0) pinMode(PIN_MULT_LED, OUTPUT);

  // Line6 lines: set the level before switching to output so nothing glitches.
  digitalWrite(PIN_L6_BANK_SELECT, LOW);
  pinMode(PIN_L6_BANK_SELECT, OUTPUT);
  digitalWrite(PIN_L6_TAP_DISABLE, HIGH);
  pinMode(PIN_L6_TAP_DISABLE, OUTPUT);
  setShiftLines(false);

  footswitch.begin(PIN_FOOTSWITCH, INPUT_PULLUP, true, DEBOUNCE_MS);
  tapInput.begin(PIN_TAP, TAP_PIN_MODE, true, DEBOUNCE_MS);

  pinMode(PIN_TEMPO_POT, INPUT);
  potSmoothed = analogRead(PIN_TEMPO_POT);
  basePeriodMs = potToPeriod(potSmoothed);

  randomSeed(analogRead(PIN_TEMPO_POT) ^ micros());
  lastMicros = micros();
}

void loop() {
  const uint32_t now = millis();
  const uint32_t us = micros();
  const float dtMs = (us - lastMicros) / 1000.0f;
  lastMicros = us;

  // Inputs
  if (tapInput.update(now) && tapInput.pressed() && !shiftOn) handleTap(now);
  pollPot(now);
  handleFootswitch(footswitch.update(now), now);

  if (shiftPulseUntil && (int32_t)(now - shiftPulseUntil) >= 0) {
    shiftPulseUntil = 0;
    setShiftLines(false);
  }

  // Engine
  const Wave w = currentWave();
  lfo.advance(dtMs, cyclePeriodMs());
  const float lfoValue = lfo.value(w, RANDOM_STEPS);
  updateLevel(dtMs, w, now);

  // Outputs
  writeOutput(level * lfoValue, now);
  updateIndicators(w, lfoValue, now);
}
