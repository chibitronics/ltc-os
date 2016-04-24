#ifndef __ORCHARD_STORAGE__
#define __ORCHARD_STORAGE__

#include "flash.h"
#include "demod.h" // for references to tx block length

// naming conventions:
// SECTORS are 1k long, and match the erase block size of the Kinetis FLASH controller
// BLOCKS are 256 bytes long, and match the size of data transmitted from the host
// STORAGE always refers to physical addresses in memory

// storage_start is defined in os/common/ports/ARMCMx/compilers/GCC/ld/KL02P20.ld

#define STORAGE_START (uint8_t *) (__storage_start__)
#define SECTOR_SIZE   (uint32_t) FTFx_PSECTOR_SIZE
#define BLOCK_SIZE    PAYLOAD_LEN

// one sector is reserved for swapping in for updates
#define STORAGE_SIZE  ((uint32_t) (__storage_size__) 

#define BLOCK_TOTAL   (STORAGE_SIZE / BLOCK_SIZE)  // number of blocks, 1-based
#define BLOCK_MAX     (BLOCK_TOTAL - 1)   // max block index, 0-based
#define SECTOR_MAX ( (uint32_t) __storage_end__ / SECTOR_SIZE)
#define SECTOR_MIN ((uint32_t) __storage_start__ / SECTOR_SIZE)
#define SECTOR_COUNT (SECTOR_MAX - SECTOR_MIN + 1)
#define SECTOR_INVALID   0xFFFFFFFF  // return coode for errors

#define STORAGE_HEADER_OFFSET  STORAGE_START
#define STORAGE_PROGRAM_OFFSET (STORAGE_START + 256)

#define STORAGE_VERSION  0
#define STORAGE_MAGIC    0x62696843  // 'Chib'

#define GUID_BYTES  16
// has to fit in 256 bytes per spec
// contains signature & management information
typedef struct storage_header_ {
  // below here is program ID
  uint32_t version;
  uint32_t magic;   // needed to differentiate blank from programmed
  uint32_t fullhash;  // hash of the program -- so we can integrity check & fail partial programs
  uint32_t length; // total length of loaded program in bytes
  uint8_t guid[GUID_BYTES]; // guid of the program contained within
  // below here is local state
  uint32_t complete; // set to 0xFFFFFFFF if not all blocks have been received and hash checks out
  uint32_t blockmap[40]; // map of blocks that have been updated; 0xFFFFFFFF if not updated
  // cur max size blockmap = 55 for storage_header to fit in 256 byte space allocated
} storage_header;

// this needs to mirror the program ID section of the storage_header
// we do this because we're in a machine with a tiny amount of stack
// and allocating a 256 byte struct for flashing is ... not going to work.
typedef struct storage_header_ram_ {
  uint32_t version;
  uint32_t magic;   // needed to differentiate blank from programmed
  uint32_t fullhash;  // hash of the program -- so we can integrity check & fail partial programs
  uint32_t length; // total length of loaded program in bytes
  uint8_t guid[GUID_BYTES]; // guid of the program contained within
} storage_header_ram;

// Hardware restrictions:
// * Pages are 1k in size
// * You can't write 0's over 0's -- it reduces flash lifetime
// * Minimum patch size is 4 bytes (one word)

// this is the call made to process an incoming packet, once it has been received
int8_t updaterPacketProcess(uint8_t *pkt);

#endif // __ORCHARD_STORAGE__
