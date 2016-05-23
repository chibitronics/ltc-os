#include "hal.h"
#include "adc.h"

#include "Arduino.h"
#include "kl02.h"
#include "memio.h"
#include "printf.h"

static int pin_to_port(int pin, ioportid_t *port, uint8_t *pad) {

  if (pin == A0) {
    *port = IOPORT2;
    *pad = 10;
    return 0;
  }

  if (pin == A1) {
    *port = IOPORT2;
    *pad = 11;
    return 0;
  }

  if (pin == A2) {
    *port = IOPORT1;
    *pad = 12;
    return 0;
  }

  if (pin == A3) {
    *port = IOPORT2;
    *pad = 12;
    return 0;
  }

  if (pin == A4) {
    *port = IOPORT2;
    *pad = 5;
    return 0;
  }

  if (pin == 0) {
    *port = IOPORT2;
    *pad = 0;
    return 0;
  }

  if (pin == 1) {
    *port = IOPORT1;
    *pad = 7;
    return 0;
  }

  return -1;
}

void pinMode(int pin, enum pin_mode arduino_mode) {

  ioportid_t port;
  uint8_t pad;
  iomode_t mode;

  if (pin_to_port(pin, &port, &pad))
    return;

  if (arduino_mode == INPUT_PULLUP)
    mode = PAL_MODE_INPUT_PULLUP;
  else if (arduino_mode == INPUT_PULLDOWN)
    mode = PAL_MODE_INPUT_PULLDOWN;
  else if (arduino_mode == INPUT)
    mode = PAL_MODE_INPUT;
  else
    mode = PAL_MODE_OUTPUT_PUSHPULL;

  palSetPadMode(port, pad, mode);
}

/* Digital IO */
void digitalWrite(int pin, int value) {

  ioportid_t port;
  uint8_t pad;
  iomode_t mode;

  if (pin_to_port(pin, &port, &pad))
    return;

  palWritePad(port, pad, !!value);
}

int digitalRead(int pin) {

  ioportid_t port;
  uint8_t pad;
  iomode_t mode;

  if (pin_to_port(pin, &port, &pad))
    return;

  return palReadPad(port, pad);
}

/* Analog IO */
void analogWrite(int pin, int value) {
  return;
}

void analogReference(enum analog_reference_type type) {
  (void)type;

  return;
}

static uint32_t pin_to_adc(int pin) {

  if (pin == A0)
    return ADC_AD9;
  if (pin == A1)
    return ADC_AD8;
  if (pin == A2)
    return ADC_DAD0;
  if (pin == A3)
    return ADC_AD13;
  if (pin == A4)
    return ADC_DAD1;
  if (pin == A5)
    return ADC_TEMP_SENSOR;
  if (pin == A6)
    return ADC_BANDGAP;
  if (pin == A7)
    return ADC_VREFSH;
  if (pin == A8)
    return ADC_VREFSL;

  return 0;
}

static void mux_as_adc(int pin) {

  ioportid_t port;
  uint8_t pad;

  if (pin_to_port(pin, &port, &pad))
    return;

  palSetPadMode(port, pad, PAL_MODE_INPUT_ANALOG);
}

int analogRead(int pin) {

  msg_t result;
  adcsample_t sample;
  ADCConversionGroup arduinogrp = {
    0, // circular buffer mode? no.
    1, // just one channel
    NULL,  // callback
    NULL,  // error callback
    pin_to_adc(pin),
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

  mux_as_adc(pin);

  result = adcConvert(&ADCD1,
                     &arduinogrp,
                     &sample,
                     1);
  if (result)
    sample = 0;

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
