#include "hal.h"
#include "kl02.h"
#include "Arduino.h"

unsigned long millis(void) {
  return ST2MS(chVTGetSystemTimeX());
}

unsigned long micros(void) {
  return ST2US(chVTGetSystemTimeX());
}

void delay(unsigned long msecs) {
  chThdSleepMilliseconds(msecs);
}

void delayMicroseconds(unsigned int usecs) {
  chThdSleepMicroseconds(usecs);
}
