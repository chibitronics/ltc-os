#include <stdint.h>
#include "hal.h"
#include "Arduino.h"
#include "arduino-io.h"

extern void ledUpdate(uint32_t num_leds, void *pixels, uint32_t mask,
                      uint32_t set_addr, uint32_t clr_addr);
void ledShow(uint32_t pin, void *pixels, uint32_t num_leds) {

  ioportid_t port;
  uint8_t pad;
  uint32_t base;

  pin = canonicalizePin(pin);

  /* Don't let users access illegal pins.*/
  if (!canUsePin(pin))
    return;

  if (pinToPort(pin, &port, &pad) < 0)
    return;

  if (port == IOPORT1)
    base = FGPIOA_BASE;
  else if (port == IOPORT1)
    base = FGPIOB_BASE;

  chSysLock();
  ledUpdate(num_leds, pixels, 1 << pad,
      base + GPIO_PSOR,
      base + GPIO_PCOR);
  chSysUnlock();
}
