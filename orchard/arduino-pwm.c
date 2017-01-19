#include "hal.h"

#include "Arduino.h"
#include "arduino-io.h"
#include "memio.h"

#define SOFT_PWM_CYCLE 64
#define SOFT_TONE

/* The rate at which the soft PWM runs */
#define SOFT_PWM_FREQUENCY 23437

/* Soft PWM for D4 and D5, set to -1 to disable. */
/* If the threshold is above these, output high. Otherwise, output low.  Set to -1 to disable*/
int32_t soft_pwm[2] = {-1, -1};

static uint16_t soft_pwm_counter = SOFT_PWM_CYCLE;

/* Soft PWM timer object */
static uint8_t soft_pwm_running;

static void (*soft_pwm_hook)(void);

/* Analog IO */
#define ARDUINO_MAX 255 /* Arduino PWM values go from 0 to 255 */
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

int softPwmTick(void) {

  /* If both timers are stopped, unhook ourselves. */
  if ((soft_pwm[0] == -1)
   && (soft_pwm[1] == -1)
   && (!soft_pwm_hook)) {
    writeb(0, SPI0_C1);
    nvicDisableVector(SPI0_IRQn);
    detachFastInterrupt(SPI_IRQ);
    soft_pwm_running = 0;
    return 1;
  }

  if (soft_pwm_counter == soft_pwm[0])
    writel((1 << 0), FGPIOB_PSOR);

  if (soft_pwm_counter == soft_pwm[1])
    writel((1 << 7), FGPIOA_PSOR);

  soft_pwm_counter--;
  if (soft_pwm_counter == 0) {
    soft_pwm_counter = SOFT_PWM_CYCLE;
    writel((1 << 7), FGPIOA_PCOR);
    writel((1 << 0), FGPIOB_PCOR);
  }

  if (soft_pwm_hook)
    soft_pwm_hook();

  /* Read the S register to reset the TIEF bit,
   * allowing transmit to happen.
   */
  (void)readb(SPI0_S);

  /* Write one byte out, to trigger the SPI, which will call us back in 1/7000 Hz.*/
  writeb(0xff, SPI0_D);

  /* Allow the normal handler to run */
  return 1;
}

static void soft_pwm_start(void) {
  if (!soft_pwm_running) {
    soft_pwm_running = 1;
    attachFastInterrupt(SPI_IRQ, softPwmTick);

    /* Ungate SPI block */
    SIM->SCGC4 |= SIM_SCGC4_SPI0;

    /* Enable the SPI system, in master mode, with an interrupt on "Empty". */
    writeb(SPIx_C1_SPE | SPIx_C1_MSTR | SPIx_C1_SPTIE, SPI0_C1);

    /* Run SPI at 50 kHz, which will give us a ~5.8 kHz ISR */
    //writeb(0x8, SPI0_BR); // Divide 47 MHz clock by 512

    /* Run SPI at 187.5 kHz, which will give us a ~23.4 kHz ISR */
    writeb(0x6, SPI0_BR); // Divide 47 MHz clock by 128

    /* Run SPI at 1.5 MHz, which will give us a 187.5 kHz ISR */
    //writeb(0x3, SPI0_BR); // Divide 47 MHz clock by 16

    /* Enable the vector only once the device has been configured */
    nvicEnableVector(SPI0_IRQn, KINETIS_SPI_SPI0_IRQ_PRIORITY);
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

  /* If the pin is an input, don't write to it.*/
  if ((palGetPadMode(port, pad) == PAL_MODE_INPUT)
   || (palGetPadMode(port, pad) == PAL_MODE_INPUT_PULLUP)
   || (palGetPadMode(port, pad) == PAL_MODE_INPUT_PULLDOWN)
   || (palGetPadMode(port, pad) == PAL_MODE_INPUT_ANALOG))
    return;

  /* Clamp the value to between 0 and 255, for the principle of least surprise */
  if (value > ARDUINO_MAX)
    value = ARDUINO_MAX;
  if (value < 0)
    value = 0;

  switch (canonicalizePin(pin)) {
  case 0:
    palSetPadMode(IOPORT1, 8, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD1;
    channel = 1;
    break;

  case 1:
    palSetPadMode(IOPORT1, 9, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD1;
    channel = 0;
    break;

  case 2:
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD2;
    channel = 0;
    break;

  case 3:
    palSetPadMode(IOPORT2, 4, PAL_MODE_UNCONNECTED);
    mode = PAL_MODE_ALTERNATIVE_2;
    driver = &PWMD2;
    channel = 1;
    break;

  case 4:
    soft_pwm[0] = (value * SOFT_PWM_CYCLE) / 256;

    /* Round up, if the value got truncated to 0 */
    if (value && !soft_pwm[0])
      soft_pwm[0] = 1;
    palSetPadMode(port, pad, PAL_MODE_OUTPUT_PUSHPULL);
    soft_pwm_start();
    return; /* Return, don't enable PWM since we're faking it */

  case 5:
    soft_pwm[1] = (value * SOFT_PWM_CYCLE) / 256;
    /* Round up, if the value got truncated to 0 */
    if (value && !soft_pwm[1])
      soft_pwm[1] = 1;
    palSetPadMode(port, pad, PAL_MODE_OUTPUT_PUSHPULL);
    soft_pwm_start();
    return; /* Return, don't enable PWM since we're faking it */

  default:
    /* Invalid channel */
    return;
  }

  palSetPadMode(port, pad, mode);

  /* Start the driver, if necessary. */
  if (driver->state != PWM_READY)
    pwmStart(driver, &pwmcfg);

  pwmEnableChannel(driver, channel, value * PWM_PERIOD / ARDUINO_MAX);

  return;
}

void hookSoftPwm(void (*new_soft_pwm_hook)(void)) {
  soft_pwm_hook = new_soft_pwm_hook;

  /* If there's actually a hook now, kick off the timer */
  if (soft_pwm_hook)
   soft_pwm_start();
}

void enableTimer(int timer_number) {
  void enableLptmr(void);
  if (timer_number == 1)
    enableLptmr();
}