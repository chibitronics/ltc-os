#include "hal.h"
#include "flash.h"


int8_t flashErase(uint32_t sectorOffset, uint16_t sectorCount) {
  return flashEraseSectors(sectorOffset,  sectorCount);
}

int8_t flashWrite(uint8_t *src, uint8_t *dst, uint32_t count) {
  return flashProgram(src, dst, count);
}