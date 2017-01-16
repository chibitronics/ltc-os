#include "hal.h"
#include "Arduino.h"

extern int (*portaFastISR)(void);
extern void (*portaISR)(void);
extern int (*portbFastISR)(void);
extern void (*portbISR)(void);
extern int (*pwm0FastISR)(void);
extern void (*pwm0ISR)(void);
extern int (*pwm1FastISR)(void);
extern void (*pwm1ISR)(void);
extern int (*adcFastISR)(void);
extern void (*adcISR)(void);
extern int (*i2cFastISR)(void);
extern void (*i2cISR)(void);
extern int (*serialFastISR)(void);
extern void (*serialISR)(void);
extern int (*lptmrFastISR)(void);
extern void (*lptmrISR)(void);

int (*spiFastISR)(void);
int (*i2c0FastISR)(void);
void (*spiISR)(void);
void (*i2c0ISR)(void);

void attachInterrupt(int irq, void (*func)(void), enum irq_mode mode) {

  (void)mode;

  switch (irq) {
    case PORTA_IRQ:
      portaISR = func;
      break;

    case PORTB_IRQ:
      portbISR = func;
      break;

    case PWM0_IRQ:
      pwm0ISR = func;
      break;

    case PWM1_IRQ:
      pwm1ISR = func;
      break;

    case ADC_IRQ:
      adcISR = func;
      break;

    case I2C_IRQ:
      i2cISR = func;
      break;

    case SERIAL_IRQ:
      serialISR = func;
      break;

    case LPTMR_IRQ:
      lptmrISR = func;
      break;

    /* Note: reuses fast ISR here */
    case I2C0_IRQ:
      i2c0ISR = func;
      break;

    /* Note: reuses fast ISR here */
    case SPI_IRQ:
      spiISR = func;
      break;
  }
  return;
}

void detachInterrupt(int irq) {
  attachInterrupt(irq, NULL, 0);
}

void attachFastInterrupt(int irq, int (*func)(void)) {
  switch(irq) {
    case PORTA_IRQ:
      portaFastISR = func;
      break;

    case PORTB_IRQ:
      portbFastISR = func;
      break;

    case PWM0_IRQ:
      pwm0FastISR = func;
      break;

    case PWM1_IRQ:
      pwm1FastISR = func;
      break;

    case ADC_IRQ:
      adcFastISR = func;
      break;

    case I2C_IRQ:
      i2cFastISR = func;
      break;

    case LPTMR_IRQ:
      lptmrFastISR = func;
      break;

    case I2C0_IRQ:
      i2c0FastISR = func;
      break;

    case SPI_IRQ:
      spiFastISR = func;
      break;
  }
  return;
}

void detachFastInterrupt(int irq) {
  attachFastInterrupt(irq, NULL);
}

int digitalPinToInterrupt(int pin) {
  ioportid_t port;
  uint8_t pad;
  pinToPort(pin, &port, &pad);

  if (port == IOPORT1)
    return PORTA_IRQ;
  else if (port == IOPORT2)
    return PORTB_IRQ;
  return NOT_AN_INTERRUPT;
}

/* SPI handler */
OSAL_IRQ_HANDLER(Vector68) {
  if (spiFastISR)
    if (!spiFastISR())
      return;
  if (spiISR)
    spiISR();
}

/* I2C2 handler */
OSAL_IRQ_HANDLER(Vector60) {
  if (i2c0FastISR)
    if (!i2c0FastISR())
      return;
  if (i2c0ISR)
    i2c0ISR();
}