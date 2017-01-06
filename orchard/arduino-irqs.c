#include "hal.h"
#include "Arduino.h"

extern void (*portaFastISR)(void);
extern void (*portaISR)(void);
extern void (*portbFastISR)(void);
extern void (*portbISR)(void);
extern void (*pwm0FastISR)(void);
extern void (*pwm0ISR)(void);
extern void (*pwm1FastISR)(void);
extern void (*pwm1ISR)(void);
extern void (*adcFastISR)(void);
extern void (*adcISR)(void);
extern void (*i2cFastISR)(void);
extern void (*i2cISR)(void);
extern void (*serialFastISR)(void);
extern void (*serialISR)(void);
extern void (*lptmrFastISR)(void);
extern void (*lptmrISR)(void);

void (*spiFastISR)(void);
void (*i2c2FastISR)(void);

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
    case I2C2_IRQ:
      i2c2FastISR = func;
      break;

    /* Note: reuses fast ISR here */
    case SPI_IRQ:
      spiFastISR = func;
      break;
  }
  return;
}

void detachInterrupt(int irq) {
  attachInterrupt(irq, NULL, 0);
}

void attachFastInterrupt(int irq, void (*func)(void)) {
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

    case I2C2_IRQ:
      i2c2FastISR = func;
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

/* SPI handler */
OSAL_IRQ_HANDLER(Vector68) {
  if (spiFastISR)
    spiFastISR();
}

/* I2C2 handler */
OSAL_IRQ_HANDLER(Vector64) {
  if (i2c2FastISR)
    i2c2FastISR();
}