// =============================================================================
//  Tap Express — configuration
//  Everything a builder is likely to want to change lives in this file.
//  Defaults match the v1.1 board (Arduino Pro Mini, 5 V / 16 MHz) and the
//  behaviour of the original v2.3 firmware.
// =============================================================================
#pragma once

// -----------------------------------------------------------------------------
//  Pins  (same as v2.3 — drop-in for the existing board)
//  Selector inputs use the internal pull-up and are pulled to GND to select.
// -----------------------------------------------------------------------------
#define PIN_TAP             1    // Tap tempo input (Line6 M5 tap switch, 3.3 V logic, external pull-up)
                                 // NOTE: pin 1 is also Serial TX — see DEBUG_SERIAL below.
#define PIN_RAMP_SEL_A      2    // Ramp-length toggle, line A
#define PIN_RAMP_SEL_B      18   // Ramp-length toggle, line B (A4)
#define PIN_MULT_SEL_A      3    // Tempo multiply toggle, line A
#define PIN_MULT_SEL_B      4    // Tempo multiply toggle, line B
#define PIN_PREVIEW_LED     5    // PWM — waveform preview (green)
#define PIN_WAVE_SEL_1      6    // Waveform rotary, bit 2
#define PIN_WAVE_SEL_2      7    // Waveform rotary, bit 1
#define PIN_WAVE_SEL_3      8    // Waveform rotary, bit 0
#define PIN_RAMP_LED        9    // PWM — ramp indicator
#define PIN_TEMPO_LED       10   // Tempo flash
#define PIN_OUTPUT          11   // PWM — drives the LED of the LED/LDR (the actual expression output)
#define PIN_FOOTSWITCH      12   // Momentary footswitch to GND (internal pull-up)
#define PIN_SHIFT_LED       13   // Shift indicator (red)
#define PIN_L6_BANK_SELECT  16   // A2 — to CD4066: presses Line6 FX + TAP together (preset select)
#define PIN_L6_TAP_DISABLE  17   // A3 — to CD4066: disconnects the tap line during preset select
#define PIN_MULT_LED        19   // A5 — declared but never driven in v2.3; v4 mirrors the tempo flash here.
                                 //      Set to -1 to leave the pin alone.
#define PIN_TEMPO_POT       A0   // Tempo pot wiper

#define TAP_PIN_MODE        INPUT   // INPUT for the M5 tap line (it has its own pull-up),
                                    // INPUT_PULLUP for a plain footswitch to GND.

// -----------------------------------------------------------------------------
//  Debug
// -----------------------------------------------------------------------------
// Serial uses pins 0/1. v2.3 printed debug output on the same pin it read the
// tap switch from, which corrupts both. Only enable this if you move PIN_TAP.
#define DEBUG_SERIAL        0
#define DEBUG_BAUD          115200

// -----------------------------------------------------------------------------
//  Output (LED/LDR)
// -----------------------------------------------------------------------------
#define INVERT_OUTPUT       0     // 1 = LED fully on when the effect is off (v2.3 "LowActiveLED")
#define OUTPUT_MIN          0     // PWM value at the bottom of the range (0–255). The "heel" position.
#define OUTPUT_MAX          255   // PWM value at the top of the range (0–255). The "toe" position.
// Response curve. 1.0 = linear PWM, identical to v2.3. LDRs are very non-linear
// (resistance drops fastest at low light), so a linear ramp tends to "jump"
// early. Values around 2.0–3.0 usually give a more even sweep — tune by ear.
#define OUTPUT_GAMMA        1.0
#define PREVIEW_MAX         127   // Brightness ceiling for the preview LED while idle

// -----------------------------------------------------------------------------
//  Waveforms
//  The rotary/selector produces a 3-bit value (pins 6,7,8; open = 1).
//  Map each value to a waveform. Values 1,2,3,5,6,7 match v2.3.
//  0 and 4 did nothing in v2.3; v4 puts a triangle there.
// -----------------------------------------------------------------------------
#define WAVE_FOR_0          WAVE_TRIANGLE
#define WAVE_FOR_1          WAVE_SINE
#define WAVE_FOR_2          WAVE_SAW_UP
#define WAVE_FOR_3          WAVE_SQUARE
#define WAVE_FOR_4          WAVE_TRIANGLE
#define WAVE_FOR_5          WAVE_SAW_DOWN
#define WAVE_FOR_6          WAVE_RANDOM
#define WAVE_FOR_7          WAVE_ON        // all switches open — the "ramp" / swell mode

#define RANDOM_STEPS        8     // New random target this many times per cycle (smoothed)

// -----------------------------------------------------------------------------
//  Tempo
// -----------------------------------------------------------------------------
#define POT_MIN_MS          60    // Shortest cycle from the tempo pot
#define POT_MAX_MS          2000  // Longest cycle from the tempo pot
#define POT_TAKEOVER        10    // ADC counts the pot must move to take control back from tap tempo
#define TAP_MIN_MS          60    // Taps closer than this are ignored (bounce)
#define TAP_MAX_MS          8000  // A gap longer than this starts a new tap sequence

// Tempo multiply toggle: 2-bit value from PIN_MULT_SEL_A/B (open = 1).
// v2.3 mapping: 1 -> x4 (slower), 2 -> x0.5 (faster), 3 (centre) -> x1.
#define MULT_FOR_0          1.0
#define MULT_FOR_1          4.0
#define MULT_FOR_2          0.5
#define MULT_FOR_3          1.0

// -----------------------------------------------------------------------------
//  Ramp (WAVE_ON only, unless RAMP_LFO_WAVES is 1)
//  Ramp length is a number of beats of the *base* tempo, so it follows tap tempo.
//  v2.3 only implemented the centre position (4 beats); the other two were empty.
// -----------------------------------------------------------------------------
#define RAMP_BEATS_FOR_0    4.0
#define RAMP_BEATS_FOR_1    2.0
#define RAMP_BEATS_FOR_2    8.0
#define RAMP_BEATS_FOR_3    4.0   // centre position — what v2.3 used
#define RAMP_MIN_MS         100
#define RAMP_MAX_MS         30000
#define RAMP_LFO_WAVES      0     // 1 = also fade the LFO waveforms in/out over the ramp time

// -----------------------------------------------------------------------------
//  Footswitch
// -----------------------------------------------------------------------------
#define DEBOUNCE_MS         20
#define LONG_PRESS_MS       500   // Hold longer than this = momentary mode
#define DOUBLE_CLICK_MS     200   // Window after a release in which a second press counts as a double-click

// -----------------------------------------------------------------------------
//  Shift / Line6 preset select
// -----------------------------------------------------------------------------
// 0 = each click in shift mode toggles the CD4066 lines (first click holds the
//     Line6 switches down, next click releases them) — exactly what v2.3 did.
// >0 = each click fires a pulse of this many ms (the v2.3 code had this
//     commented out).
#define SHIFT_PULSE_MS      0

// -----------------------------------------------------------------------------
//  Sanity checks
// -----------------------------------------------------------------------------
#if DEBUG_SERIAL && (PIN_TAP == 0 || PIN_TAP == 1)
#error "DEBUG_SERIAL uses pins 0/1, which clashes with PIN_TAP. Move the tap input or disable DEBUG_SERIAL."
#endif
#if OUTPUT_MIN >= OUTPUT_MAX
#error "OUTPUT_MIN must be lower than OUTPUT_MAX"
#endif
