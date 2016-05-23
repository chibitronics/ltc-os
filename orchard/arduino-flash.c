#include "hal.h"
#include "flash.h"


int8_t flashErase(uint32_t sectorOffset, uint16_t sectorCount) {
  uint8_t ret;

  osalSysLock();
  ret = flashEraseSectors(sectorOffset,  sectorCount);
  osalSysUnlock();
  return ret;
}

int8_t flashWrite(uint8_t *src, uint8_t *dst, uint32_t count) {
  uint8_t ret;

  osalSysLock();
  ret = flashProgram(src, dst, count);
  osalSysUnlock();
  return ret;
}
