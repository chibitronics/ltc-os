#include "hal.h"

#include "esplanade_demod.h"
#include "esplanade_mac.h"
#include "flash.h"
#include "esplanade_updater.h"
#include "murmur3.h"

#include "orchard.h"
#include "printf.h"

#include "esplanade_app.h"
#include "esplanade_os.h"

#include <string.h>

/*
  Facts:
  -  The KL02 has 1k sectors.
  -  Data is marshalled in 256-byte blocks.
     - The 256-byte blocks are protected with a murmur3 hash check
     - This isn't cryptographically good, but probably good enough to make
       collisions due to random bit flips more rare than say, cables falling out
       or power loss during programming.
  -  Users will typically be using phones to program the stickers. It's not possible
     to guarantee silence on the headphone port, so a sound-based trigger to initiate
     programming is ruled out (imagine if every time you get a notification, your
     project flips into programming mode; pretty annoying). 
  
  Transmission format:
  - Two types of blocks: data, and control
    - Data block consists of: 
      -- Preamble Sync (00, 00, 00, 00, aa, 55, 42)
      -- Version code (1 byte: 0x00-0x7f)
      -- Block offset (2 bytes): address offset = (block offset * 256 bytes + app code base)
      -- Data (256 bytes)
      -- Hash check (4 bytes -- covers version code through end of data)
    - Control block consists of:
      -- Preamble Sync (00, 00, 00, 00, aa, 55, 42)
      -- Version code (1 byte: 0x80-0xff)
      -- Total block count for this transmission (2 bytes)
      -- Final code region hash (4 bytes)
      -- md5sum of code region (20 bytes) -- used to ID programs

  Memory allocation:
    note: code defensively so these limits can be easily changed anytime from now until spec freeze
    spec freeze happens upon first production run
  00k
    22k for Bootstrapping ocde
  22k
    256 bytes for signature & management
  22.25k
    256 bytes for interrupt vector relocation table (via VTOR)
  22.5k
    9.5k for User application code (including any arduino libraries that have to stay resident)
  32k
 
  Programming Algorithm:
  - Unit boots to programing mode and searches with a timeout of 5s for a carrier tone;
    failing a carrier, it will revert to run mode if and only if a valid program already exists
    *** THIS MEANS COMPILE AND PROGRAM BUTTONS MUST BE DIFFERENT ON WEB UI *** as compiles could
    take longer than 5 seconds
  - Once a valid block is received:
    - Upon receipt of control block, check that md5 of sending block is different from current block
      (to avoid playback loops from reprogramming the system over and over again)
    - Upon receipt of unique md5, /all/ of program flash is erased
    - The blocks are written to flash based on received address until all expected blocks are received
    - System exits program mode once all blocks are received and starts running user code

  New proposed programming method:
  - Assume the presence of an external circuit which takes an external button, and provides two 
    views on it: a pulsed version, and the current level
    -- the pulsed version is wired to RESET
    -- the current level is fed into a GPIO
  - On boot or reset, check the current level of the external button. 
    - If it's held down for more than 1s, go into programming mode, and stay there.
    - Otherwise, go into application run mode and never come back

  Thus, press-and-hold to program; tap to reset to app; and also, power cycle to reset to app.
  Also, once program is successful, automatically start the app. woot!
 */

typedef enum states {
  APP_IDLE      = 0,
  APP_GOT_ID    = 1, // (unused state)
  APP_UPDATING  = 2, // keep circulating here until all blocks received
  APP_UPDATED   = 3, // check if all blocks are good in this state
  APP_FAIL      = 4, // app failed to boot
} app_state;
static app_state astate = APP_IDLE;

bl_symbol(static const storage_header *storageHdr) =
    (const storage_header *) STORAGE_START;

void bootToUserApp(void) {
  tfp_printf( "\n\r Reached boot to user app!!!\n\r" );
  GPIOB->PCOR |= (1 << 6);   // blue on
  /*
    todo:
      -- reset ADC subsystem to prevent samples from triggering interrupts
      -- reset any other initialized subsystems (i2c, etc.)
      -- set VTOR
      -- soft reset
   */
  if (appIsValid())
    chThdExit(0);
  else
    printf("But app is not valid.\r\n");
}

void init_storage_header(demod_pkt_ctrl_t *cpkt) {
  storage_header_ram proto;

  proto.version   = STORAGE_VERSION;
  proto.magic     = STORAGE_MAGIC;
  proto.fullhash  = cpkt->fullhash;
  proto.length    = cpkt->length;
  memcpy(proto.guid, cpkt->guid, GUID_BYTES);

  // this routine could fail, but...nothing to do if it doesn't work!
  flashProgram((uint8_t *)&proto,
               (uint8_t *)storageHdr,
               sizeof(proto));
}

// guarantee entering here: all packets are "good" (as in they pass mac-level hash checks)
// this state machine really fucks up flash if you pass random garbage into it:
// it'll repeatedly erase flash due to guid mismatch fails!
// we also assume the packets are the correct version; the MAC should reject packets for
// versions that don't match our firmware
int8_t updaterPacketProcess(demod_pkt_t *pkt) {

  demod_pkt_ctrl_t *cpkt;
  demod_pkt_data_t *dpkt;
  int8_t err = 0;
  uint32_t i;

  tfp_printf("S%d ", (uint8_t) astate);
  switch (astate) {
  case APP_IDLE:
    cpkt = &pkt->ctrl_pkt; // expecting a control packet
    
    if (cpkt->header.type != PKTTYPE_CTRL)
      break; // if not a control packet, stay in idle

    /* we don't check the magic #, just guid because chance of collision
     * is remote.
     */
    if (memcmp(storageHdr->guid, cpkt->guid, 16) == 0) {
      if (storageHdr->complete == 0xFFFFFFFF) {
        /* we're getting a resend of an incomplete transmission, move
         * to the updating state.
         */
        astate = APP_UPDATING;
        break;
      }
      else {
        break; // attempt to program the sticker with the same program, just abort & ignore
      }
    }

    /* We've got a control packet, and it's for a new program guid.
     * Nuke the flash to make room for the new code and pray that
     * the update doesn't fail.
     */
    err = flashEraseSectors(SECTOR_MIN, SECTOR_COUNT);

    /* Re-initialize the storage header. */
    init_storage_header(cpkt);
    astate = APP_UPDATING;
    break;

  case APP_UPDATING:

    /* We should /only/ get here if the header has been initialized!!
     * If Magic has changed, there has been some kind of corruption to
     * the internal header.  Reset the system to a known state.
     */
    if (storageHdr->magic != STORAGE_MAGIC) {
      err = flashEraseSectors(SECTOR_MIN, SECTOR_COUNT);
      astate = APP_IDLE;
      break;
    }

    dpkt = &pkt->data_pkt;
    if (dpkt->header.type != PKTTYPE_DATA)
      break; // if not a data packet, ignore and wait again

    /* Check and see if the current sector we're trying to write
     * has been updated before flashing it.  It's bad for Flash 
     * to write over a sector that already has data.
     */
    uint16_t block = dpkt->block;
    if (storageHdr->blockmap[block] == 0xFFFFFFFF) {
      /* NOTE: we first clear the block map before programming because
       * if someone powers down the system in the middle of the
       * block programming, we don't want to accidentally reprogram
       * the block, as this will overstress the flash.
       *
       * There's a full-program hash check later on that will save us
       * from any partially programmed blocks later on.....
       */
      uint32_t dummy = 0;

      /* clear the entry in the block map to record programming state. */
      err = flashProgram((uint8_t *)&dummy, (uint8_t *)(&(storageHdr->blockmap[block])), sizeof(uint32_t));
      tfp_printf( "\n\r P%d b%d", (uint8_t) block, err );
      
      // only program if the blockmap says it's not been programmed
      err = flashProgram(dpkt->payload, (uint8_t *) (STORAGE_PROGRAM_OFFSET + (block * BLOCK_SIZE)), BLOCK_SIZE);
      tfp_printf( " d%d", err );
    }
    else {
      tfp_printf( " _%d", (uint8_t) block ); // redundant block received
    }
    
    /* Now check if the entire block map, within the range of the
     * program length, has been programmed.  We want to do this
     * on every block, even if it's already been programmed, because
     * there's a potential race condition where we could have received
     * the last block, but failed to blow the "complete" flag, due to
     * a power failure at the wrong time.  In other words, don't
     * make the below check an "else" clause of the previous "if",
     * thinking it's an optimization.
     */
    uint8_t alldone = 1;
    for (i = 0; i < ((storageHdr->length - 1) / BLOCK_SIZE) + 1; i++) {
      if (storageHdr->blockmap[i] == 0xFFFFFFFF)
        alldone = 0;
    }
    if(!alldone)
      break;  // stay in app-updating state

    /* Now that it's claimed to be done, do a full hash check
     * and confirm this /actually/ worked.
     */
    uint32_t hash;
    MurmurHash3_x86_32((uint8_t *)STORAGE_PROGRAM_OFFSET, storageHdr->length, MURMUR_SEED_TOTAL, &hash);
    if (hash == storageHdr->fullhash) {
      /* Hurray, we're done! mark the whole thing as complete. */
      uint32_t dummy = 0;
      err = flashProgram((uint8_t *)(&(storageHdr->complete)), (uint8_t *)&dummy, sizeof(uint32_t));
      astate = APP_UPDATED;
      bootToUserApp();
      astate = APP_FAIL;
    }
    else {
      tfp_printf( "\n\r Transfer complete but corrupted. Erase & retry.\n\r" );
      tfp_printf( "\n\r Source hash: %08x local hash: %08x\n\r", storageHdr->fullhash, hash );
      
      /* Hash check failed. Something went wrong. Just nuke all of storage
       * and bring us back to a virgin state.
       */
      err = flashEraseSectors(SECTOR_MIN, SECTOR_COUNT);
      astate = APP_IDLE;
    }
    break;

  case APP_UPDATED:
    bootToUserApp();
    astate = APP_FAIL;
    break;
    
  default:
    break;
  }

  pktReady = 0;

  return err; // sort of bogus because I only return the last flash error encountered...
}
