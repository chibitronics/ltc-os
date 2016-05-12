#include <stdint.h>
#include "hal.h"
#include "Arduino.h"

extern void ledUpdate(uint32_t num_leds, void *pixels, uint32_t mask,
                      uint32_t set_addr, uint32_t clr_addr);
void ledShow(uint32_t pin, void *pixels, uint32_t num_leds) {

  chSysLock();
  ledUpdate(num_leds, pixels, digitalPinToBitMask(pin),
      digitalPinToPort(pin) + GPIO_PSOR,
      digitalPinToPort(pin) + GPIO_PCOR);
  chSysUnlock();
}
