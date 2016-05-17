#include "hal.h"
#include "adc.h"

#include "Arduino.h"
#include "kl02.h"
#include "memio.h"
#include "printf.h"

#define PIN_NUMBER(x) (x & 0x1f)
#define BANK_NUMBER(x) ((x & ~0x1f) >> 5)

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

int pinToADC(int pin) {
  /* Microphone input channel. */
  return ADC_DADP0;
}

int analogRead(int pin) {

  msg_t result;
  static const ADCConversionGroup arduinogrp = {
    0, // circular buffer mode? no.
    1, // just one channel
    NULL,  // callback
    NULL,  // error callback
    pinToADC(pin),
    // CFG1 register
    // SYSCLK = 48MHz.
    // BUSCLK = SYSCLK / 4 = 12MHz
    // ADCCLK = SYSCLK / 2 / 1 = 6 MHz

    // ADLPC = 0 (normal power)
    // ADLSMP = 0 (short sample time)
    // ADHSC = 0 (normal conversion sequence)
    // -> 5 ADCK cycles + 5 bus clock cycles = SFCadder
    // 20 ADCK cycles per sample

    ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_2) |
    ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
    ADCx_CFG1_MODE(ADCx_CFG1_MODE_12_OR_13_BITS),  // 12 bits per sample

    // SC3 register
    ADCx_SC3_AVGE |
    ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_4_SAMPLES) // 4 sample average

    //      48 MHz sysclk
    // /2   24 MHz busclk
    // /2   12 MHz after prescaler
    // /2   6 MHz after adiv
    // /20  300ksps after base sample time @ 12 bps
    // /4   75ksps after averaging by factor of 4
  };

  adcsample_t sample;
  result = adcConvert(&ADCD1,
                     &arduinogrp,
                     &sample,
                     1);
  return sample;
}


void tone(int pin, unsigned int frequency, unsigned long duration) {

  return;
}

void noTone(int pin) {

  return;
}

/* Simple communication protocols */
void shiftOut(int dataPin, int clockPin, int bitOrder, uint8_t val) {

  uint8_t i;

  for (i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));

    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}

uint8_t shiftIn(int dataPin, int clockPin, int bitOrder) {

  uint8_t value = 0;
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    digitalWrite(clockPin, HIGH);
    if (bitOrder == LSBFIRST)
      value |= digitalRead(dataPin) << i;
    else
      value |= digitalRead(dataPin) << (7 - i);
    digitalWrite(clockPin, LOW);
  }
  return value;
}

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout) {
  uint32_t startMicros = micros();
  state = !!state;

  /* Wait for previous pulse to end */
  while (digitalRead(pin) == state)
    if (micros() - startMicros > timeout)
      return 0;

  /* Wait for the pulse to start */
  while (digitalRead(pin) != state)
    if (micros() - startMicros > timeout)
      return 0;

  /* Wait for the pulse to end */
  while (digitalRead(pin) == state)
    if (micros() - startMicros > timeout)
      return 0;

  return micros() - startMicros;
}

unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout) {
  return pulseIn(pin, state, timeout);
}
