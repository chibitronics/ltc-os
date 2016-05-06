#include "Arduino.h"
#include "kl02.h"
#include "memio.h"

#define PIN_NUMBER(x) (x & 0x1f)
#define BANK_NUMBER(x) ((x & 0xe0) >> 5)

static int pin_to_hwpin(int pin) {
  return pin;
}

void pinMode(int pin, enum pin_mode mode) {

  pin = pin_to_hwpin(pin);

  switch (BANK_NUMBER(pin)) {
  case 0: /* PTA */
    /* Set pin mux to GPIO, and set pull */
    if (mode == INPUT_PULLUP)
      writel(0x00000103, PORTA_PCR(PIN_NUMBER(pin)));
    else if (mode == INPUT_PULLDOWN)
      writel(0x00000102, PORTA_PCR(PIN_NUMBER(pin)));
    else
      writel(0x00000100, PORTA_PCR(PIN_NUMBER(pin)));

    /* Set pin direction */
    if (mode == OUTPUT)
      writel(readl(GPIOA_PDDR) | (1 << PIN_NUMBER(pin)), GPIOA_PDDR);
    else
      writel(readl(GPIOA_PDDR) & ~(1 << PIN_NUMBER(pin)), GPIOA_PDDR);

    break;

  case 1: /* PTB */
    /* Set pin mux to GPIO, and set pull */
    if (mode == INPUT_PULLUP)
      writel(0x00000103, PORTB_PCR(PIN_NUMBER(pin)));
    else if (mode == INPUT_PULLDOWN)
      writel(0x00000102, PORTB_PCR(PIN_NUMBER(pin)));
    else
      writel(0x00000100, PORTB_PCR(PIN_NUMBER(pin)));

    /* Set pin direction */
    if (mode == OUTPUT)
      writel(readl(GPIOB_PDDR) | (1 << PIN_NUMBER(pin)), GPIOB_PDDR);
    else
      writel(readl(GPIOB_PDDR) & ~(1 << PIN_NUMBER(pin)), GPIOB_PDDR);
    break;

  }
}

/* Digital IO */
void digitalWrite(int pin, int value) {
  pin = pin_to_hwpin(pin);
  switch (BANK_NUMBER(pin)) {

  case 0:
    writel(1 << PIN_NUMBER(pin), value ? FGPIOA_PSOR : FGPIOA_PCOR);
    break;

  case 1:
    writel(1 << PIN_NUMBER(pin), value ? FGPIOB_PSOR : FGPIOB_PCOR);
    break;
  }
}

int digitalRead(int pin) {
  pin = pin_to_hwpin(pin);
  switch (BANK_NUMBER(pin)) {
  case 0: return !!(readl(FGPIOA_PDIR) & (1 << PIN_NUMBER(pin)));
  case 1: return !!(readl(FGPIOB_PDIR) & (1 << PIN_NUMBER(pin)));
  }
  return 0;
}

/* Analog IO */
void analogWrite(int pin, int value) {
  return;
}

void analogReference(enum analog_reference_type type) {
  return;
}

int analogRead(int pin) {
  return 0;
}


void tone(int pin, unsigned int frequency, unsigned long duration) {
  return;
}

void noTone(int pin) {
  return;
}

/* Simple communication protocols */
void shiftOut(int dataPin, int clockPin, int bitOrder, uint8_t val) {
  return;
}

uint8_t shiftIn(int dataPin, int clockPin, int bitOrder) {
  return 0;
}

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout) {
  return 0;
}

unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout) {
  return 0;
}
