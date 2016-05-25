#include "hal.h"
#include "kl02.h"
#include "Arduino.h"

systime_t app_start_time;

unsigned long millis(void) {
  return ST2MS(chVTTimeElapsedSinceX(app_start_time));
}

unsigned long micros(void) {
  return ST2US(chVTTimeElapsedSinceX(app_start_time));
}

void delay(unsigned long msecs) {
  chThdSleepMilliseconds(msecs);
}

void delayMicroseconds(unsigned int usecs) {
  chThdSleepMicroseconds(usecs);
}
