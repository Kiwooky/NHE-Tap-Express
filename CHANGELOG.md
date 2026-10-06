# Changelog

## v4.0 — 2026

Full rewrite of v2.3. Same pinout, same controls, same ramp behaviour.

### Changed
- **Non-blocking engine.** The ramps used `delay()` loops, which froze tap tempo, the tempo pot and the LEDs while they ran. Now everything runs together.
- **No libraries.** JLed and OneButton have been replaced by a small phase-accumulator LFO (`Lfo.h`) and a debounced input (`Input.h`). Tempo changes no longer restart the waveform objects, so they're glitch-free.
- **Footswitch logic is now one explicit state machine.** Previously it was spread across click, long-press and double-click callbacks.
- **All settings are in `config.h`:** pins, waveform map, multiply and ramp tables, output range and curve.

### Fixed
- **The ramp-length toggle now works.** In v2.3 only the centre position did anything.
- **The ramp time now follows the tempo.** In v2.3 it was only recalculated when you flipped the ramp toggle, so in practice it was fixed at about 8 s.
- **The ×4 multiply** no longer makes the tempo LED blink 4× faster while the effect runs 4× slower. The factor was also applied twice to the pot range.
- **Long gaps between taps no longer produce a garbage tempo.** `MAXTAP` was defined but never used, and the 16-bit `period` overflowed after about 32 s.
- **The tap input on pin 1 (Serial TX)** no longer fights with the debug output. Serial is now off by default, and the build refuses to enable it while the tap is on pin 0 or 1.
- **The tap line is ignored in shift mode**, so preset-select presses don't set a bogus tempo.
- **Leaving shift mode** now releases the Line6 lines instead of possibly leaving them held down.
- **Mid-ramp reversals** respond to a press straight away. v2.3 had a 300 ms debounce pause at each direction change.

### Added
- **MIDI CC out** on A1: the same output as a CC, alongside the LDR or instead of it. You can set the channel, controller, range and inversion, and there's an optional 14-bit mode. It uses a built-in transmit-only MIDI driver (bit timing checked in simavr to within 1% at 8 and 16 MHz) because the hardware TX pin is taken by the tap input.
- **Triangle wave** on the spare selector positions (0 and 4 did nothing before)
- **`OUTPUT_MIN` / `OUTPUT_MAX` / `OUTPUT_GAMMA`** to shape the sweep for your LDR and pedal input
- **`RAMP_LFO_WAVES`** to fade the LFO waveforms in and out over the ramp time
- **`SHIFT_PULSE_MS`**, a pulse mode for preset select (the v2.3 code had it commented out)
- **The tempo flash is mirrored on pin 19**, the multiply LED, which v2.3 declared but never drove
- **Host-side simulation tests** and a GitHub Actions build

## v2.3 — June 2021 (Pro Mini, M5 board v1.1)
The last 2021 version. See `legacy/` and `docs/v2.3-review.md`.

## v1.2 – v2.2, v3.0 — May–June 2021
Development versions, kept unchanged in `legacy/`.
