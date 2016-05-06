#include "kl02.h"
#include "Arduino.h"

unsigned long millis(void) {
  return 0;
}

unsigned long micros(void) {
  return 0;
}

void delay(unsigned long msecs) {
  int i;
  for (i = 0; i < 1000000; i++)
    asm("nop");
}

void delayMicroseconds(unsigned int usecs) {
  int i;
  for (i = 0; i < 100; i++)
    asm("nop");
}
