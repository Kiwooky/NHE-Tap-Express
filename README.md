# Tap Express

**A tap-tempo LFO and footswitch ramp for any expression-pedal input.**

Tap Express is an Arduino that pretends to be your foot. It drives an LED/LDR (a homemade vactrol) sitting on your multi-effect's expression input, so the LDR stands in for the pedal's potentiometer. Pick a waveform, tap a tempo, and the "pedal" moves by itself. Or hold the footswitch and swell the effect in and out.

It was built for a **Line6 M5**: it takes tap tempo straight from the M5's tap switch and can jump to the M5's preset screen. Anything with a TRS expression input should work, though you may need to tune the resistor that sits in parallel with the LDR.

> Status: v4.0 is a clean rewrite of the 2021 v2.3 firmware. It runs on the same board as v2.3. It compiles and passes a host-side simulation of the switch and ramp logic, but **it hasn't been flashed to the hardware yet**. If you build one, please open an issue and tell us how it went.

---

## Features

- **7 waveforms:** sine, triangle, ramp-up saw, ramp-down saw, square, smoothed random, and constant ON
- **Ramp / swell** (constant ON mode): the footswitch fades the output from off to fully on over a set number of beats, and you can reverse it at any point mid-ramp
- **Tap tempo** from a footswitch or the Line6 tap switch, plus a **tempo pot**. Whichever you touched last wins.
- **Multiply toggle:** ×½ / ×1 / ×4 (effectively double speed, as tapped, or quarter speed)
- **Ramp-length toggle:** 2 / 4 / 8 beats, so the ramp follows the tempo
- **Momentary or latching:** hold the switch or tap it
- **Shift mode** (double-tap) to call up the Line6 preset select through a CD4066
- **Preview LED** shows the selected waveform before you engage it
- **No external libraries.** All settings live in one file, [`config.h`](firmware/TapExpress/config.h).

## How the footswitch works

| Action | LFO waveforms | Constant ON (ramp mode) |
|---|---|---|
| **Tap** | Latches the waveform on. Tap again to stop it. | Ramps up on its own and **latches on** at the top. Tap again to ramp down. |
| **Hold** (> 0.5 s) | Runs the waveform while you hold it | Ramps up while you hold. **It stays on only while you hold.** Release to ramp down. |
| **Press mid-ramp** | – | Reverses direction (both modes) |
| **Release mid-ramp** (hold mode) | – | Reverses back down to off |
| **Double-tap** (from off) | Toggles shift mode (red LED) | same |
| **Tap in shift mode** | Triggers Line6 preset select | same |

## Hardware

- Arduino Pro Mini, 5 V / 16 MHz (any ATmega328P board works if you change the pins)
- LED + LDR in a light-tight tube (or a ready-made vactrol), plus a resistor or trimmer in parallel with the LDR to set the "off" resistance (the "EXP POT" on the board)
- Momentary footswitch
- 6-position rotary switch (or DIP switches) for the waveform: 3 lines, pulled to ground
- Two on/off/on toggles, one for multiply and one for ramp length
- 10k linear pot for tempo
- Optional: CD4066 quad switch for the Line6 tap and preset-select tricks

The stripboard layout for the M5 build is in [`docs/Tap Express M5 board v1.1.pdf`](docs/).

### Pinout (v1.1 board)

| Pin | Function | Pin | Function |
|---|---|---|---|
| 1 | Tap input (Line6 tap line) | 10 | Tempo LED |
| 2 | Ramp select A | 11 | **Output → LED/LDR** (PWM) |
| 3 | Multiply select A | 12 | Footswitch |
| 4 | Multiply select B | 13 | Shift LED (red) |
| 5 | Preview LED (green, PWM) | A0 | Tempo pot wiper |
| 6, 7, 8 | Waveform select bits | A2 (16) | Line6 preset select → CD4066 |
| 9 | Ramp LED (PWM) | A3 (17) | Line6 tap disable → CD4066 |
| | | A4 (18) | Ramp select B |
| | | A5 (19) | Multiply / tempo LED |

Waveform select (pins 6/7/8, open = 1):

| Value | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 (all open) |
|---|---|---|---|---|---|---|---|---|
| Wave | triangle | sine | saw up | square | triangle | saw down | random | constant ON / ramp |

## Building and flashing

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) or `arduino-cli`.
2. Open `firmware/TapExpress/TapExpress.ino`.
3. Board: **Arduino Pro or Pro Mini**. Processor: **ATmega328P (5V, 16 MHz)**.
4. Check `config.h`, then upload with a USB-serial (FTDI) adapter.

```sh
arduino-cli compile --fqbn arduino:avr:pro:cpu=16MHzatmega328 firmware/TapExpress
arduino-cli upload  --fqbn arduino:avr:pro:cpu=16MHzatmega328 -p /dev/ttyUSB0 firmware/TapExpress
```

## Tuning (`config.h`)

| Setting | What it does |
|---|---|
| `OUTPUT_MIN` / `OUTPUT_MAX` | Heel and toe limits of the sweep (0–255) |
| `OUTPUT_GAMMA` | Response curve. 1.0 is linear like v2.3. Try 2–3 if the sweep bunches up at one end, because LDRs are very non-linear. |
| `INVERT_OUTPUT` | Fully on when the effect is off. Use this if your pedal input works the other way round. |
| `POT_MIN_MS` / `POT_MAX_MS` | Tempo pot range |
| `MULT_FOR_*`, `RAMP_BEATS_FOR_*` | What each toggle position does |
| `WAVE_FOR_*` | Which waveform sits on each selector position |
| `RAMP_LFO_WAVES` | Also fade the LFO waveforms in and out over the ramp time |
| `SHIFT_PULSE_MS` | 0 toggles the Line6 lines (v2.3 behaviour). > 0 sends a pulse instead. |
| `TAP_PIN_MODE` | `INPUT` for the M5 tap line, `INPUT_PULLUP` for a plain footswitch |

> **Serial debug:** the tap input is on pin 1, which is also Serial TX. `DEBUG_SERIAL` is off by default, and the build stops with an error if you turn it on without moving the tap pin.

## Tests

The switch, ramp and tempo logic runs on a PC against a small Arduino stub:

```sh
g++ -std=c++11 -Itest/stub -Ifirmware/TapExpress test/sim_test.cpp -o sim && ./sim
```

GitHub Actions runs this test and compiles the firmware for the Pro Mini on every push.

## Repository layout

```
firmware/TapExpress/   v4 firmware (open this in the Arduino IDE)
test/                  host-side simulation tests
docs/                  board layout, v2.3 review notes
legacy/                the original 2021 sketches, v1.2 to v2.3, unchanged
```

## History

Tap Express started in May 2021 as a tap-tempo expression modulator for a Line6 M5. It went through v1.2 to v2.3 over a few weeks, with an experimental v3.0 branch that started on MIDI. v4.0 (2026) rewrites the v2.3 behaviour without blocking loops or libraries. See the [CHANGELOG](CHANGELOG.md) and [`docs/v2.3-review.md`](docs/v2.3-review.md).

## Ideas / roadmap

- Real min/max depth pots on the free analog pins (A6/A7)
- Store the last tempo in EEPROM
- MIDI clock in (the v3.0 branch started on this)
- One-shot envelopes on the spare selector positions

Pull requests welcome.

## License

MIT. See [LICENSE](LICENSE).
