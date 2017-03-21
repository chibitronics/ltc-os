#include "hal.h"
#include "adc.h"
#include "Arduino.h"
#include "kl02.h"
#include "memio.h"

#include "esplanade_app.h" // for app_heap

/* Whether users can access "advanced" features, such as Red/Green LEDs.*/
static uint8_t sudo_mode;

extern int32_t soft_pwm[2];

/* Pins that can be used when not in "sudo" mode.*/
static const uint8_t normal_mode_pins[] = {
  0, 1, 2, 3, 4, 5,
};

uint32_t palGetPadMode(ioportid_t port, uint8_t pad) {
  PORT_TypeDef *portcfg = NULL;

  if (port == IOPORT1)
    portcfg = PORTA;
  else if (port == IOPORT2)
    portcfg = PORTB;
  else
    return (uint32_t)-1;

  switch (portcfg->PCR[pad] & PORTx_PCRn_MUX_MASK) {
  case PORTx_PCRn_MUX(0):
    return PAL_MODE_INPUT_ANALOG;
  case PORTx_PCRn_MUX(2):
    return PAL_MODE_ALTERNATIVE_2;
  case PORTx_PCRn_MUX(3):
    return PAL_MODE_ALTERNATIVE_3;
  case PORTx_PCRn_MUX(4):
    return PAL_MODE_ALTERNATIVE_4;
  case PORTx_PCRn_MUX(5):
    return PAL_MODE_ALTERNATIVE_5;
  case PORTx_PCRn_MUX(6):
    return PAL_MODE_ALTERNATIVE_6;
  case PORTx_PCRn_MUX(7):
    return PAL_MODE_ALTERNATIVE_7;
  case PORTx_PCRn_MUX(1):
    if (port->PDDR & ((uint32_t) 1 << pad))
      return PAL_MODE_OUTPUT_PUSHPULL;

    switch (portcfg->PCR[pad] & (PORTx_PCRn_PE | PORTx_PCRn_PS)) {
    case (PORTx_PCRn_PE | PORTx_PCRn_PS):
      return PAL_MODE_INPUT_PULLUP;
    case PORTx_PCRn_PE:
      case PAL_MODE_INPUT_PULLDOWN:
    default:
      return PAL_MODE_INPUT;
    }
  default:
    return PAL_MODE_UNCONNECTED;
  }
}

void arduinoIoInit(void) {
  return;
}

int canonicalizePin(int pin) {

  int masked_pin = pin;

  /* Convert D0..D15 to 0..15.*/
  if ((masked_pin & 0xf0) == 0xa0)
    masked_pin &= ~0xf0;

  /* Convert A0..A15 to 0..15.*/
  if ((masked_pin & 0xf0) == 0x80)
    masked_pin &= ~0xf0;

  switch (masked_pin) {
  case PTB(10):
  case PTA(8):
  case LED_BUILTIN:
  case 0:
    return 0;

  case PTB(11):
  case PTA(9):
  case 1:
    return 1;

  case PTA(12):
  case 2:
    return 2;

  case PTB(13):
  case PTB(4):
  case 3:
    return 3;

  case PTB(0):
  case 4:
    return 4;

  case PTA(7):
  case 5:
    return 5;

  case PTA(5):
    return LED_BUILTIN_RED;

  case PTA(6):
    return LED_BUILTIN_RGB;

  case PTB(6):
    return LED_BUILTIN_GREEN;

  case PTB(5):
    return AUDIO_IN;

  case PTB(2):
    return UART_TX;

  case PTB(1):
    return UART_RX;

  case PTA(0):
    return SWD_CLK;

  case PTA(2):
    return SWD_DIO;

  case PTB(7):
    return RST_LEVEL;

  default:
    return pin;
  }
}

int pinToPort(int pin, ioportid_t *port, uint8_t *pad) {

  switch(canonicalizePin(pin)) {
  case 0:
    *port = IOPORT2;
    *pad = 10;
    break;

  case 1:
    *port = IOPORT2;
    *pad = 11;
    break;

  case 2:
    *port = IOPORT1;
    *pad = 12;
    break;

  case 3:
    *port = IOPORT2;
    *pad = 13;
    break;

  case 4:
    *port = IOPORT2;
    *pad = 0;
    break;

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

  case RST_LEVEL:
    *port = IOPORT2;
    *pad = 7;
    break;

  default:
    if ((pin & 0xf0) == 0x40) {
      *port = IOPORT1;
      *pad = pin & 0x1f;
      break;
    }
    else if ((pin & 0xf0) == 0x60) {
      *port = IOPORT2;
      *pad = pin & 0x1f;
      break;
    }
    return -1;
  }

  return 0;
}

int canUsePin(int pin) {

  unsigned int i;

  if (sudo_mode)
    return 1;

  pin = canonicalizePin(pin);
  for (i = 0; i < sizeof(normal_mode_pins); i++)
    if (normal_mode_pins[i] == pin)
      return 1;

  return 0;
}

void pinMode(int pin, enum pin_mode arduino_mode) {

  ioportid_t port;
  uint8_t pad;
  iomode_t mode;

  if (pinToPort(pin, &port, &pad) < 0)
    return;

  /* Don't let users access illegal pins.*/
  if (!canUsePin(pin))
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

  if (pinToPort(pin, &port, &pad) < 0)
    return;

  /* Don't let users access illegal pins.*/
  if (!canUsePin(pin))
    return;

  switch (palGetPadMode(port, pad)) {
  /* If we Write to a pin that's an input, set/clear the pullup instead.
   * See documentation for digitalWrite(). */
  case PAL_MODE_INPUT_PULLDOWN:
  case PAL_MODE_INPUT:
  case PAL_MODE_INPUT_PULLUP:
    if (value)
      palSetPadMode(port, pad, PAL_MODE_INPUT_PULLUP);
    else
      palSetPadMode(port, pad, PAL_MODE_INPUT);
    return; /* Return, and don't set a new value.*/

  /* Mux this pin as an output, if it's set as ALTERNATIVE_2 (i.e. PWM) */
  case PAL_MODE_ALTERNATIVE_2:
    palSetPadMode(port, pad, PAL_MODE_OUTPUT_PUSHPULL);
    break;
  }

  /* Disable the running PWM, if one exists */
  if ((pin == D4) || (pin == A4) || (pin == 4))
    soft_pwm[0] = -1;
  if ((pin == D5) || (pin == A5) || (pin == 5))
    soft_pwm[1] = -1;

  /* Write the value out the pad.*/
  palWritePad(port, pad, !!value);
}

int digitalRead(int pin) {

  ioportid_t port;
  uint8_t pad;

  if (pinToPort(pin, &port, &pad) < 0)
    return 0;

  switch(palGetPadMode(port, pad)) {
    case PAL_MODE_INPUT_ANALOG:
      palSetPadMode(port, pad, PAL_MODE_INPUT);
    case PAL_MODE_INPUT:
    case PAL_MODE_INPUT_PULLUP:
    case PAL_MODE_INPUT_PULLDOWN:
      break;
    default:
      return 0;
  }

    /* Don't let users access illegal pins.*/
  if (!canUsePin(pin))
    return 0;

  return palReadPad(port, pad) ? HIGH : LOW;
}

void analogReference(enum analog_reference_type type) {
  (void)type;

  return;
}

static int pin_to_adc(int pin) {

  switch (canonicalizePin(pin)) {
  case 0:
    return ADC_AD9;

  case 1:
    return ADC_AD8;

  case 2:
    return ADC_DAD0;

  case 3:
    return ADC_AD13;

  case 4:
    return ADC_AD6;

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

/* Analog stuff */
#define SYSTEM_ANALOG_RESOLUTION 12
/* Default to 10 bits for Arduino compatibility */
static uint8_t analog_read_resolution = 10;

void analogReadResolution(int bits) {
  if ((bits < 0) || (bits > 31))
    return;

  analog_read_resolution = bits;
}

int analogRead(int pin) {

  msg_t result;
  adcsample_t sample;
  int adc_num;
  ioportid_t port;
  uint8_t pad;

  /* If we have a valid pin here, ensure it's muxed as an input
   * or as an analog device.*/
  if (pinToPort(pin, &port, &pad) >= 0) {
    switch(palGetPadMode(port, pad)) {
    case PAL_MODE_INPUT:
    case PAL_MODE_INPUT_PULLUP:
    case PAL_MODE_INPUT_PULLDOWN:
      palSetPadMode(port, pad, PAL_MODE_INPUT_ANALOG); /* fall through */
    case PAL_MODE_INPUT_ANALOG:
      break;
    default:
      return 0;
    }
  }

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


  result = adcConvert(&ADCD1,
                     &arduinogrp,
                     &sample,
                     1);
  if (result)
    sample = 0;

  // Shift the result up or down to match the requested resolution.
  if (analog_read_resolution > SYSTEM_ANALOG_RESOLUTION)
    sample <<= (analog_read_resolution - SYSTEM_ANALOG_RESOLUTION);
  else if (analog_read_resolution < SYSTEM_ANALOG_RESOLUTION)
    sample >>= (SYSTEM_ANALOG_RESOLUTION - analog_read_resolution);

  return sample;
}

void doSudo(void) {
  sudo_mode = 1;
}

void setSerialSpeed(uint32_t speed) {
  ioportid_t port;
  uint8_t pad;
  SerialConfig serialConfig = {
    speed,
  };

  sdStop(&SD1);

  /* On boot, we mux the two pins to GPIO outputs and drive
   * them low, to simulate USB disconnect.
   * Since we're setting the serial speed now, we obviously
   * want to use the pins as part of a serial port.
   * Remux them as UART.
   */

  pinToPort(UART_TX, &port, &pad);
  palSetPadMode(port, pad, PAL_MODE_ALTERNATIVE_3);

  pinToPort(UART_RX, &port, &pad);
  palSetPadMode(port, pad, PAL_MODE_ALTERNATIVE_3);

  sdStart(&SD1, &serialConfig);
}
