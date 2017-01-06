#include "hal.h"
#include "adc.h"
#include "Arduino.h"
#include "kl02.h"
#include "memio.h"
#include "printf.h"
#include "lptmr.h"

#include "esplanade_app.h" // for app_heap

/* Soft PWM for D4 and D5, set to -1 to disable. */
/* If the threshold is above these, output high. Otherwise, output low.  Set to -1 to disable*/
static int32_t soft_pwm[2] = {-1, -1};

/* Soft PWM timer object */
static thread_t *soft_pwm_thread;

/* Virtual timer for disabling the tone object */
virtual_timer_t tone_timer;

/* Whether users can access "advanced" features, such as Red/Green LEDs.*/
static uint8_t sudo_mode;

/* Pins that can be used when not in "sudo" mode.*/
static const uint8_t normal_mode_pins[] = {
  0, 1, 2, 3, 4, 5,
  A0, A1, A2, A3, A4, A5,
  D0, D1, D2, D3, D4, D5,
  LED_BUILTIN,
};

void arduinoIoInit(void) {
  chVTObjectInit(&tone_timer);
}

int pinToPort(int pin, ioportid_t *port, uint8_t *pad) {

  switch(pin) {

  case LED_BUILTIN:
  case A0:
  case D0:
  case 0:
    *port = IOPORT2;
    *pad = 10;
    break;

  case A1:
  case D1:
  case 1:
    *port = IOPORT2;
    *pad = 11;
    break;

  case A2:
  case D2:
  case 2:
    *port = IOPORT1;
    *pad = 12;
    break;

  case A3:
  case D3:
  case 3:
    *port = IOPORT2;
    *pad = 13;
    break;

  case A4:
  case D4:
  case 4:
    *port = IOPORT2;
    *pad = 0;
    break;

  case A5:
  case D5:
  case 5:
    *port = IOPORT1;
    *pad = 7;
    break;

  case LED_BUILTIN_RGB:
    *port = IOPORT1;
    *pad = 6;
    break;

  case UART_TX:
    *port = IOPORT2;
    *pad = 2;
    break;

  case UART_RX:
    *port = IOPORT2;
    *pad = 1;
    break;

  case SWD_CLK:
    *port = IOPORT1;
    *pad = 0;
    break;

  case SWD_DIO:
    *port = IOPORT1;
    *pad = 2;
    break;

  case LED_BUILTIN_RED:
    *port = IOPORT1;
    *pad = 5;
    break;

  case LED_BUILTIN_GREEN:
    *port = IOPORT2;
    *pad = 6;
    break;

  case AUDIO_IN:
    *port = IOPORT2;
    *pad = 5;
    break;

  default:
    return -1;
  }

  return 0;
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

  if (pinToPort(pin, &port, &pad))
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
    palSetPadMode(IOPORT2, 4, PAL_MODE_UNCONNECTED);

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

  if (pinToPort(pin, &port, &pad))
    return;

  /* Don't let users access illegal pins.*/
  if (!can_use_pin(pin))
    return;

  /* Disable the running PWM, if one exists */
  if ((pin == D4) || (pin == A4) || (pin == 4))
    soft_pwm[0] = -1;
  if ((pin == D5) || (pin == A5) || (pin == 5))
    soft_pwm[1] = -1;

  palWritePad(port, pad, !!value);
}

int digitalRead(int pin) {

  ioportid_t port;
  uint8_t pad;

  if (pinToPort(pin, &port, &pad))
    return 0;

  /* Don't let users access illegal pins.*/
  if (!can_use_pin(pin))
    return 0;

  return palReadPad(port, pad) ? HIGH : LOW;
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

#define SOFT_PWM_CYCLE 64
#define SOFT_PWM_CYCLE_MULTIPLIER 16

static uint32_t soft_pwm_counter = SOFT_PWM_CYCLE;

void softPwmTick(void) {

  /* If both timers are stopped, unhook ourselves. */
  if ((soft_pwm[0] == -1) && (soft_pwm[1] == -1)) {
    extern void (*lptmrFastISR)(void);
    stopLptmr(void);
    lptmrFastISR = NULL;
  }

  if (soft_pwm_counter <= soft_pwm[0])
    writel((1 << 0), FGPIOB_PSOR);

  if (soft_pwm_counter <= soft_pwm[1])
    writel((1 << 7), FGPIOA_PSOR);

  soft_pwm_counter--;
  if (soft_pwm_counter == 0) {
    soft_pwm_counter = SOFT_PWM_CYCLE;
    writel((1 << 7), FGPIOA_PCOR);
    writel((1 << 0), FGPIOB_PCOR);
  }

  /* Clear the TCF bit, which lets the timer continue.*/
  writel(LPTMR_CSR_TCF | LPTMR_CSR_TIE | LPTMR_CSR_TEN, LPTMR0_CSR);
}

static void soft_pwm_start(void) {
  extern void (*lptmrFastISR)(void);
  if (!soft_pwm_thread) {
    soft_pwm_thread = (void *)1;
    lptmrFastISR = softPwmTick;

    /* The "pin" and "port" numbers don't matter, since we're hooking the ISR */
    startLptmr(0, 0, 5000);
  }
}

void analogWrite(int pin, int value) {

  ioportid_t port;
  uint8_t pad;
  uint8_t channel;
  iomode_t mode;
  PWMDriver *driver = NULL;

  if (pinToPort(pin, &port, &pad))
    return;

  switch (pin) {
  case A0:
  case D0:
  case 0:
  case LED_BUILTIN:
    palSetPadMode(IOPORT1, 8, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD1;
    channel = 1;
    break;

  case A1:
  case D1:
  case 1:
    palSetPadMode(IOPORT1, 9, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD1;
    channel = 0;
    break;

  case A2:
  case D2:
  case 2:
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD2;
    channel = 0;
    break;

  case A3:
  case D3:
  case 3:
    palSetPadMode(IOPORT2, 4, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD2;
    channel = 1;
    break;

  case A4:
  case D4:
  case 4:
    if (value > 255)
      value = 255;
    if (value < 0)
      value = 0;
    soft_pwm[0] = (value * SOFT_PWM_CYCLE) / 256;
    soft_pwm_start();
    return; /* Return, don't enable PWM since we're faking it */

  case A5:
  case D5:
  case 5:
    if (value > 255)
      value = 255;
    if (value < 0)
      value = 0;
    soft_pwm[1] = (value * SOFT_PWM_CYCLE) / 256;
    soft_pwm_start();
    return; /* Return, don't enable PWM since we're faking it */

  default:
    /* Invalid channel */
    return;
    break;
  }

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

  switch (pin) {
  case A0:
  case D0:
  case 0:
    return ADC_AD9;

  case A1:
  case D1:
  case 1:
    return ADC_AD8;

  case A2:
  case D2:
  case 2:
    return ADC_DAD0;

  case A3:
  case D3:
  case 3:
    return ADC_AD13;

  case A4:
  case D4:
  case 4:
    return ADC_AD6;

  case A5:
  case D5:
  case 5:
    return ADC_AD7;

  case A6:
    return ADC_TEMP_SENSOR;

  case A7:
    return ADC_BANDGAP;

  case A8:
    return ADC_VREFSH;

  case A9:
    return ADC_VREFSL;

  case A10:
    return ADC_DAD1;

  default:
    return -1;
  }
}

static void mux_as_adc(int pin) {

  ioportid_t port;
  uint8_t pad;

  if (pinToPort(pin, &port, &pad))
    return;

  palSetPadMode(port, pad, PAL_MODE_INPUT_ANALOG);
}

int analogRead(int pin) {

  msg_t result;
  adcsample_t sample;
  int adc_num;

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

  // Ignore the return value, as it may not have a real pin,
  // e.g. if it's measuring the bandgap, or temperature.
  mux_as_adc(pin);

  result = adcConvert(&ADCD1,
                     &arduinogrp,
                     &sample,
                     1);
  if (result)
    sample = 0;

  return sample;
}

/* Timer callback.  Called from a virtual timer to stop the tone after
 * a certain duration.
 */
static void stop_tone(void *par) {

  noTone((uint32_t)par);
}

void tone(int pin, unsigned int frequency, unsigned long duration) {


  ioportid_t port;
  uint8_t pad;

  if (pinToPort(pin, &port, &pad))
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
