// Tap Express — minimal transmit-only MIDI (31250 baud, 8N1) on any digital pin.
// The Pro Mini's only hardware UART TX is pin 1, which is the tap input on the
// v1.1 board, so this bit-bangs the bytes instead. No library needed.
// Each byte takes 320 µs with interrupts paused; PWM outputs are unaffected.
#pragma once
#include <Arduino.h>

#if defined(__AVR__)
#include <util/delay_basic.h>
#endif

class MidiOut {
 public:
  void begin(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);   // idle = high
#if defined(__AVR__)
    port_ = portOutputRegister(digitalPinToPort(pin));
    mask_ = digitalPinToBitMask(pin);
#endif
  }

  void controlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    write(0xB0 | ((channel - 1) & 0x0F));
    write(cc & 0x7F);
    write(value & 0x7F);
  }

  void write(uint8_t b) {
#if defined(__AVR__)
    uint8_t sreg = SREG;
    cli();
    sendBit(false);                          // start bit
    for (uint8_t i = 0; i < 8; i++) {    // data, LSB first
      sendBit(b & 1);
      b >>= 1;
    }
    sendBit(true);                           // stop bit
    SREG = sreg;
#else
    extern void simMidiByte(uint8_t);    // host-side test hook
    simMidiByte(b);
#endif
  }

 private:
#if defined(__AVR__)
  // One bit = F_CPU / 31250 cycles (512 at 16 MHz). _delay_loop_2 burns 4
  // cycles per count; ~36 cycles go to the port write and loop (measured in
  // simavr: within 1% of 31250 baud at both 8 and 16 MHz).
  static const uint16_t BIT_LOOPS = (F_CPU / 31250UL - 36) / 4;

  inline void sendBit(bool high) {
    if (high) *port_ |= mask_;
    else      *port_ &= ~mask_;
    _delay_loop_2(BIT_LOOPS);
  }

  volatile uint8_t* port_ = nullptr;
  uint8_t mask_ = 0;
#endif
};
