#include "hal.h"

#include "orchard.h"
#include "printf.h"

#include "flash.h"
#include "esplanade_os.h"
#include "memio.h"
#include "kl02.h"

/* Commands */
#define FTFx_FSTAT_CCIF     (1 << 7)
#define FTFx_FSTAT_RDCOLERR (1 << 6)
#define FTFx_FSTAT_ACCERR   (1 << 5)
#define FTFx_FSTAT_FPVIOL   (1 << 4)
#define FTFx_FSTAT_MGSTAT0  (1 << 0)

#define FTFx_CMD_BLOCKSTAT  0x00
#define FTFx_CMD_SECTSTAT   0x01
#define FTFx_CMD_PROGCHECK  0x02
#define FTFx_CMD_LWORDPROG  0x06
#define FTFx_CMD_SECTERASE  0x09
#define FTFx_CMD_SECTWRITE  0x0b
#define FTFx_CMD_MASSERASE  0x44
#define FTFx_CMD_PGMPART    0x80
#define FTFx_CMD_SETFLEXRAM 0x81

#define MAKE_FCCOB(a, b, c, d) (\
  (((a) << 24) & 0xff000000) | \
  (((b) << 16) & 0x00ff0000) | \
  (((c) <<  8) & 0x0000ff00) | \
  (((d) <<  0) & 0x000000ff) \
  )

/* Uncomment this to store this function in RAM */
/*__attribute__((section("ramtext")))*/
static int kinetis_flash_cmd(uint8_t cmd, uint32_t addr,
                             uint32_t data1, uint32_t data2) {
  uint8_t fstat;
  int ret = F_ERR_OK;

  /* Wait for (optional) previous command to complete */
  chSysLock();
  while (!(readb(FTFx_FSTAT) & FTFx_FSTAT_CCIF))
    ;

  /* Reset error flags, if present */
  if (readb(FTFx_FSTAT) & (FTFx_FSTAT_ACCERR | FTFx_FSTAT_FPVIOL))
    writeb(FTFx_FSTAT_ACCERR | FTFx_FSTAT_FPVIOL, FTFx_FSTAT);

  /* Add in the command, which hangs out in the address top word */
  addr &= 0xffffff;
  addr |= (uint32_t)cmd << 24;

  /* Load the command bytes */
  writel(addr, FTFx_FCCOB3);
  writel(data1, FTFx_FCCOB7);
  writel(data2, FTFx_FCCOBB);

  /* start command */
  writeb(FTFx_FSTAT_CCIF, FTFx_FSTAT);

  /* Wait for the command to complete */
  while (!(readb(FTFx_FSTAT) & FTFx_FSTAT_CCIF))
    ;

  fstat = readb(FTFx_FSTAT);
  chSysUnlock();

  /* Check ACCERR and FPVIOL are zero in FSTAT */
  if (fstat & (FTFx_FSTAT_ACCERR | FTFx_FSTAT_FPVIOL | FTFx_FSTAT_MGSTAT0))
    ret = F_ERR_LOWLEVEL;

  return ret;
}

// offset is in sectors
// sectorCount is in sectors
int8_t flashEraseSectors(uint32_t offset, uint16_t sectorCount) {
  uint32_t destination;          // sector number of target
  uint32_t end;
  uint16_t number;               /* Number of longword or phrase to be program or verify*/
  uint32_t margin_read_level;    /* 0=normal, 1=user - margin read for reading 1's */
  int32_t retval = F_ERR_OK;
  
  destination = offset;
  end = destination + (uint32_t) sectorCount;

  if (destination < F_USER_SECTOR_START) {
    printf("User sectors start at %d, aborting.\r\n", F_USER_SECTOR_START);
    retval = F_ERR_RANGE;
    return retval;
  }

  if (end > ((P_FLASH_BASE + P_FLASH_SIZE) / FTFx_PSECTOR_SIZE)) {
    tfp_printf( "Too many sectors requested, end at %d but we only have %d sectors.\n\r",
	     end, (P_FLASH_BASE + P_FLASH_SIZE) / FTFx_PSECTOR_SIZE);
    retval = F_ERR_RANGE;
    return retval;
  }
  
  tfp_printf( "ES%d-%d\n\r", destination, end - 1);
  
  while (destination < end) {

    retval = kinetis_flash_cmd(FTFx_CMD_SECTERASE,
                               destination * FTFx_PSECTOR_SIZE,
                               0, 0);
    if (F_ERR_OK != retval)
      return retval;

    for (margin_read_level = 0; margin_read_level <= 2; margin_read_level++) {
      number = FTFx_PSECTOR_SIZE / 4;
      retval = kinetis_flash_cmd(FTFx_CMD_SECTSTAT,
                                 destination * FTFx_PSECTOR_SIZE,
                  MAKE_FCCOB(number >> 8, number, margin_read_level, 0),
                  0);

      if (retval)
        printf("Sector %d, margin %d, result: %d\r\n", destination, margin_read_level, retval);
    }

    tfp_printf( " e%d ", destination );
    destination++;
  }

  tfp_printf( "\n\r");
  
  return retval;
}

void flashStart(void) {

  return;
}

uint32_t flashGetSecurity(void) {
  return readb(FTFx_FSEC) & 0x3;
}

// src, dst are pointers to physical memory locations, not sectors
// count is in bytes
int8_t flashProgram(uint8_t *src, uint8_t *dest, uint32_t count) {

  uint32_t ret = F_ERR_OK;
  uint32_t failaddr = 0;
  uint32_t i;

  // do nothing if our count is 0
  if (count == 0) 
    return ret;
  
  // check if dest, dest+count is in the user-designated area of FLASH
  if ( ((uint32_t) dest < (F_USER_SECTOR_START * FTFx_PSECTOR_SIZE)) ||
      (((uint32_t) dest + count) > (P_FLASH_BASE + P_FLASH_SIZE)) ) {
    return F_ERR_RANGE;
  }

  // check if number of bytes to write, src, destination are word-aligned
  if (((count % 4) != 0)
  || ((((uint32_t) dest) % 4) != 0)
  || ((((uint32_t) src) % 4) != 0))
    return F_ERR_NOTALIGN;

  // check that the destination bytes have been erased
  // we can't re-program over 0's, it will overstress the Flash
  // (per user manual spec)
  for (i = 0; i < count; i++) {
    if (dest[i] != 0xFF)
      return F_ERR_NOTBLANK;
  }

  for (i = 0; i < count; i += 4) {
    ret = kinetis_flash_cmd(FTFx_CMD_LWORDPROG,
                            (uint32_t)&dest[i],
                            *((uint32_t *)&src[i]), 0);
  }
  
  if (ret)
    return ret;

  // user margin is more strict than normal margin -- data can be read still if
  // this fails, but indicates flash is wearing out
  for (i = 0; i < count; i += 4) {
    ret = kinetis_flash_cmd(FTFx_CMD_PROGCHECK,
                            (uint32_t)&dest[i],
                            MAKE_FCCOB(READ_USER_MARGIN, 0, 0, 0),
                            *((uint32_t *)&src[i]));
    if (ret)
      failaddr = (uint32_t)&dest[i];
  }
  
  if (ret) {
    tfp_printf("Failed programming verification at USER margin levels: worry a little bit. Failure address: %08x\n\r", failaddr );
    return F_ERR_U_MARGIN;
  }

  return ret;
}
