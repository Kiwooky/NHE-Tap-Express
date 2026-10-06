// Host simulation of the Tap Express v4 state machine.
// Build: g++ -std=c++11 -Itest/stub -Ifirmware/TapExpress test/sim_test.cpp -o sim && ./sim
#include <Arduino.h>
int simPin[32]; int simPwm[32]; int simAnalog[32]; uint32_t simMicros = 0;
#include <vector>
std::vector<uint8_t> simMidi;
void simMidiByte(uint8_t b) { simMidi.push_back(b); }
#include "../firmware/TapExpress/TapExpress.ino"

static int failures = 0;
#define CHECK(cond, msg) do { if (cond) printf("  ok   %s\n", msg); else { printf("  FAIL %s\n", msg); failures++; } } while (0)

static void run(uint32_t ms) { for (uint32_t i = 0; i < ms; i++) { simMicros += 1000; loop(); } }
static void fs(bool down) { simPin[PIN_FOOTSWITCH] = down ? LOW : HIGH; }
static void tap(uint32_t holdMs = 50) { simPin[PIN_TAP] = LOW; run(holdMs); simPin[PIN_TAP] = HIGH; }
static int out() { return simPwm[PIN_OUTPUT]; }
static void setWave(int v) { simPin[PIN_WAVE_SEL_1] = (v >> 2) & 1; simPin[PIN_WAVE_SEL_2] = (v >> 1) & 1; simPin[PIN_WAVE_SEL_3] = v & 1; }

int main() {
  for (int i = 0; i < 32; i++) simPin[i] = HIGH;      // pull-ups: everything open
  simAnalog[PIN_TEMPO_POT] = 230;                      // ~500 ms beat -> 4 beats = ~2 s ramp
  simMicros = 1000000;
  setup();
  run(100);
  printf("base period %u ms, ramp %.0f ms\n", (unsigned)basePeriodMs, rampMs());
  float R = rampMs();

  printf("\n[1] ON mode, hold through full ramp, then release\n");
  fs(true); run(400);
  CHECK(out() == 0, "nothing before long-press threshold");
  run(200 + (uint32_t)R);
  CHECK(out() == 255 && state == MOMENTARY, "reaches full while held");
  run(2000);
  CHECK(out() == 255, "stays full while held");
  fs(false); run((uint32_t)(R / 2));
  CHECK(out() > 60 && out() < 200, "ramping down after release");
  run((uint32_t)R);
  CHECK(out() == 0 && state == IDLE, "back to off and idle");

  printf("\n[2] Hold, release mid-ramp -> reverses to off\n");
  fs(true); run(520 + (uint32_t)(R * 0.4));
  int peak = out();
  CHECK(peak > 60 && peak < 160, "part-way up");
  fs(false); run(80);   // 20 ms debounce + some ramp-down
  CHECK(out() < peak, "direction reversed on release");
  run((uint32_t)R);
  CHECK(out() == 0 && state == IDLE, "ended off");

  printf("\n[3] Tap -> latches on at top, tap again -> ramps off\n");
  fs(true); run(100); fs(false); run(260);
  CHECK(state == LATCHED, "click engages latching");
  run((uint32_t)R + 50);
  CHECK(out() == 255, "reached full");
  run(5000);
  CHECK(out() == 255, "still latched 5 s later with switch released");
  fs(true); run(60); fs(false); run((uint32_t)R + 100);
  CHECK(out() == 0 && state == IDLE, "second tap ramps down to off");

  printf("\n[4] Latched, tap mid-ramp-up -> reverses\n");
  fs(true); run(100); fs(false); run(250 + (uint32_t)(R * 0.5));
  peak = out();
  fs(true); run(60); fs(false); run(50);
  CHECK(out() < peak, "reversed by mid-ramp tap");
  run((uint32_t)R);
  CHECK(out() == 0 && state == IDLE, "ended off");

  printf("\n[5] Momentary: release at top, re-press mid ramp-down -> back up and hold\n");
  fs(true); run(600 + (uint32_t)R); fs(false); run((uint32_t)(R * 0.5));
  int mid = out();
  fs(true); run(60);
  CHECK(out() > mid, "re-press reverses back up");
  run((uint32_t)R + 1000);
  CHECK(out() == 255 && state == MOMENTARY, "held at full");
  fs(false); run((uint32_t)R + 100);
  CHECK(out() == 0 && state == IDLE, "release -> off");

  printf("\n[6] Shift: double-click, preset-select click, double-click out\n");
  fs(true); run(60); fs(false); run(80); fs(true); run(60); fs(false); run(300);
  CHECK(shiftOn && simPin[PIN_SHIFT_LED] == HIGH && out() == 0, "shift on, LED lit, effect off");
  fs(true); run(60); fs(false); run(300);
  CHECK(simPin[PIN_L6_BANK_SELECT] == HIGH && simPin[PIN_L6_TAP_DISABLE] == LOW, "click toggles Line6 lines (v2.3 behaviour)");
  uint32_t before = basePeriodMs; tap(); run(300); tap(); run(100);
  CHECK(basePeriodMs == before, "taps ignored in shift mode");
  fs(true); run(60); fs(false); run(80); fs(true); run(60); fs(false); run(300);
  CHECK(!shiftOn && simPin[PIN_L6_BANK_SELECT] == LOW && simPin[PIN_L6_TAP_DISABLE] == HIGH, "shift off restores Line6 lines");

  printf("\n[7] Tap tempo, long gap, pot takeover\n");
  tap(); run(350); tap(); run(350); tap(); run(10);
  CHECK(basePeriodMs >= 395 && basePeriodMs <= 405 && !potControls, "two 400 ms taps -> 400 ms beat");
  run(20000); tap(); run(10);
  CHECK(basePeriodMs >= 395 && basePeriodMs <= 405, "tap after 20 s gap starts a new sequence (no garbage tempo)");
  simAnalog[PIN_TEMPO_POT] = 232; run(200);
  CHECK(!potControls, "pot jitter does not steal tempo");
  simAnalog[PIN_TEMPO_POT] = 700; run(300);
  CHECK(potControls && basePeriodMs > 1300, "moving the pot takes over");

  printf("\n[8] Sine wave latched: follows the LFO, x0.5 multiply\n");
  simAnalog[PIN_TEMPO_POT] = 230; run(300);
  setWave(1);
  simPin[PIN_MULT_SEL_A] = HIGH; simPin[PIN_MULT_SEL_B] = LOW;   // value 2 -> x0.5
  float cyc = cyclePeriodMs();
  CHECK(cyc > 240 && cyc < 260, "x0.5 halves the cycle");
  fs(true); run(100); fs(false); run(250);
  int mn = 255, mx = 0;
  for (int i = 0; i < 1000; i++) { run(1); if (out() < mn) mn = out(); if (out() > mx) mx = out(); }
  CHECK(mn < 5 && mx > 250, "full-range sine while latched");
  fs(true); run(60); fs(false); run(10);
  CHECK(out() == 0 && state == IDLE, "LFO waves switch off instantly (no ramp), like v2.3");

  printf("\n[9] MIDI CC mirrors the output\n");
  setWave(7); simPin[PIN_MULT_SEL_A] = HIGH; simPin[PIN_MULT_SEL_B] = HIGH;
  run(500); simMidi.clear();
  run(1000);
  CHECK(simMidi.empty(), "nothing sent while the value doesn't change");
  float R9 = rampMs();
  fs(true); run(100); fs(false); run(250);
  size_t startIdx = simMidi.size();
  uint32_t t0 = millis(); run((uint32_t)R9 + 100); uint32_t span = millis() - t0;
  bool wellFormed = (simMidi.size() - startIdx) % 3 == 0;
  int msgs = 0, lastVal = -1; bool monotonic = true;
  for (size_t i = startIdx; i + 2 < simMidi.size() + 0; i += 3) {
    if (simMidi[i] != (0xB0 | (MIDI_CHANNEL - 1)) || simMidi[i + 1] != MIDI_CC) wellFormed = false;
    if (simMidi[i + 2] < lastVal) monotonic = false;
    lastVal = simMidi[i + 2]; msgs++;
  }
  CHECK(wellFormed, "messages are CC on the configured channel/controller");
  CHECK(monotonic && lastVal == 127, "ramp up sends a rising CC ending at 127");
  CHECK(msgs <= (int)(span / MIDI_INTERVAL_MS) + 1 && msgs > 60, "rate-limited but smooth");
  CHECK(out() == 255, "LDR output still driven alongside MIDI");
  fs(true); run(60); fs(false); run((uint32_t)R9 + 100);
  CHECK(simMidi.size() >= 3 && simMidi.back() == 0 && state == IDLE, "ramp down ends with CC 0");

  printf("\n%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED", failures, failures == 1 ? "" : "s");
  return failures ? 1 : 0;
}
