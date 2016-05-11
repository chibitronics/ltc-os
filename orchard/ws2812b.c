#include <stdint.h>

extern void ledUpdate(uint32_t pin, void *pixels, uint32_t num_leds);
void ledShow(uint32_t pin, void *pixels, uint32_t num_leds) {
  ledUpdate(pin, pixels, num_leds);
}
