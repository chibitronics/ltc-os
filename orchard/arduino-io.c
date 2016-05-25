#include "hal.h"
#include "adc.h"

#include "Arduino.h"
#include "kl02.h"
#include "memio.h"
#include "printf.h"
#include "lptmr.h"

/* Whether users can access "advanced" features, such as Red/Green LEDs.*/
static int sudo_mode;

/* Pins that can be used when not in "sudo" mode.*/
static uint8_t normal_mode_pins[] = {
  D0, D1,
  A0, A1, A2, A3,
  LED_BUILTIN,
  BUTTON_A1, BUTTON_REC, BUTTON_A3,
};

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
    *pad = 13;
    return 0;
  }

  if (pin == A4) {
    *port = IOPORT2;
    *pad = 5;
    return 0;
  }

  if (pin == D0) {
    *port = IOPORT2;
    *pad = 0;
    return 0;
  }

  if (pin == D1) {
    *port = IOPORT1;
    *pad = 7;
    return 0;
  }

  if (pin == LED_BUILTIN_RGB) {
    *port = IOPORT1;
    *pad = 6;
    return 0;
  }

  if (pin == LED_BUILTIN_RED) {
    *port = IOPORT1;
    *pad = 5;
    return 0;
  }

  if (pin == LED_BUILTIN_GREEN) {
    *port = IOPORT2;
    *pad = 6;
    return 0;
  }

  if (pin == LED_BUILTIN) {
    *port = IOPORT2;
    *pad = 13;
  }

  if (pin == BUTTON_A1) {
    *port = IOPORT1;
    *pad = 9;
    return 0;
  }

  if (pin == BUTTON_A3) {
    *port = IOPORT2;
    *pad = 13;
    return 0;
  }

  if (pin == BUTTON_REC) {
    *port = IOPORT2;
    *pad = 1;
    return 0;
  }

  if (pin == UART_TX) {
    *port = IOPORT2;
    *pad = 3;
    return 0;
  }

  if (pin == UART_RX) {
    *port = IOPORT2;
    *pad = 4;
    return 0;
  }

  if (pin == SWD_CLK) {
    *port = IOPORT1;
    *pad = 0;
    return 0;
  }

  if (pin == SWD_DIO) {
    *port = IOPORT1;
    *pad = 2;
    return 0;
  }

  return -1;
}

static int can_use_pin(int pin) {

  unsigned int i;

  if (sudo_mode)
    return 1;

  for (i = 0; i < sizeof(normal_mode_pins); i++)
    if (normal_mode_pins[i] == pin)
      return 1;

  return 0;
}

void pinMode(int pin, enum pin_mode arduino_mode) {

  ioportid_t port;
  uint8_t pad;
  iomode_t mode;

  if (pin_to_port(pin, &port, &pad))
    return;

  /* Don't let users access illegal pins.*/
  if (!can_use_pin(pin))
    return;

  /* Disconnect alternate pins for A0, A1, and A3 */
  if (pin == A0)
    palSetPadMode(IOPORT1, 8, PAL_MODE_UNCONNECTED);
  if (pin == A1)
    palSetPadMode(IOPORT1, 9, PAL_MODE_UNCONNECTED);
  if (pin == A3)
    palSetPadMode(IOPORT2, 2, PAL_MODE_UNCONNECTED);

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

  if (pin_to_port(pin, &port, &pad))
    return;

  /* Don't let users access illegal pins.*/
  if (!can_use_pin(pin))
    return;

  palWritePad(port, pad, !!value);
}

int digitalRead(int pin) {

  ioportid_t port;
  uint8_t pad;

  if (pin_to_port(pin, &port, &pad))
    return 0;

  /* Don't let users access illegal pins.*/
  if (!can_use_pin(pin))
    return 0;

  return palReadPad(port, pad);
}

/* Analog IO */
#define ARDUINO_MAX 1024 /* Arduino PWM values go from 0 to 1024 */
#define PWM_DIVISOR 16
#define PWM_FREQUENCY (KINETIS_SYSCLK_FREQUENCY / PWM_DIVISOR)
#define PWM_PERIOD (PWM_FREQUENCY / (2 * ARDUINO_MAX)) /* Two cycles per seocnd */
static const PWMConfig pwmcfg = {
  PWM_FREQUENCY,                            /* 500 Hz PWM clock frequency.   */
  PWM_PERIOD,                                 /* Initial PWM period 1S.       */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
  },
};

void analogWrite(int pin, int value) {

  ioportid_t port;
  uint8_t pad;
  uint8_t channel;
  iomode_t mode;
  PWMDriver *driver = NULL;

  if (pin_to_port(pin, &port, &pad))
    return;

  /* Allow people to specify analogWrite(0) instead of analogWrite(A0).*/
  if (pin <= 8)
    pin |= 0x80;

  if (pin == A0) {
    palSetPadMode(IOPORT1, 8, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD1;
    channel = 1;
  }
  else if (pin == A1) {
    palSetPadMode(IOPORT1, 9, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD1;
    channel = 0;
  }
  else if (pin == A2) {
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD2;
    channel = 0;
  }
  else if (pin == A3) {
    palSetPadMode(IOPORT2, 2, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD2;
    channel = 1;
  }
  else
    /* Invalid channel */
    return;

  palSetPadMode(port, pad, mode);

  /* Start the driver, if necessary. */
  if (driver->state != PWM_READY)
    pwmStart(driver, &pwmcfg);

  pwmEnableChannel(driver, channel, value * PWM_PERIOD / ARDUINO_MAX);

  return;
}

void analogReference(enum analog_reference_type type) {
  (void)type;

  return;
}

static int pin_to_adc(int pin) {

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

  return -1;
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
  int adc_num;

  /* Allow people to e.g. specify analogRead(0) instead of analogRead(A0).*/
  if (pin <= 8)
    pin |= 0x80;

  adc_num = pin_to_adc(pin);

  if (adc_num == -1)
    return 0;

  ADCConversionGroup arduinogrp = {
    0, // circular buffer mode? no.
    1, // just one channel
    NULL,  // callback
    NULL,  // error callback
    adc_num,
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

virtual_timer_t tone_timer;

/* Timer callback.  Called from a virtual timer to stop the tone after
 * a certain duration.
 */
static void stop_tone(void *par) {

  noTone((uint32_t)par);
}

void tone(int pin, unsigned int frequency, unsigned long duration) {


  ioportid_t port;
  uint8_t pad;

  if (pin_to_port(pin, &port, &pad))
    return;

  /* Don't let users access illegal pins.*/
  if (!can_use_pin(pin))
    return;

  /* Ensure the pin is an output */
  palSetPadMode(port, pad, PAL_MODE_OUTPUT_PUSHPULL);

  /* Start up the low-power timer, which directly drives the port. */
  startLptmr(port, pad, frequency);

  /* Set up a timer callback to stop the tone. */
  chVTSet(&tone_timer, MS2ST(duration), stop_tone, (void *)pin);

  return;
}

void noTone(int pin) {

  (void)pin;
  stopLptmr();
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

void doSudo(void) {
  sudo_mode = 1;
}
