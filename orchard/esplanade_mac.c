#include "hal.h"
#include "esplanade_mac.h"
#include "esplanade_os.h"

// mac state is just local to mac
typedef enum mac_state {
  MAC_IDLE = 0,  
  MAC_SYNC = 1,
  MAC_PACKET = 2,
} mac_state_t;

bl_symbol_bss(volatile uint8_t *pkt_ptr);

__attribute__((aligned(4))) bl_symbol_bss(demod_pkt_t pkt);

/* Global flag to indicate packet done. needs mutex if threaded. */
bl_symbol_bss(uint8_t  pktReady);

bl_symbol(static int g_bitpos = 9);
bl_symbol_bss(static uint8_t g_curbyte);

/* Current state machine's state */
bl_symbol(static mac_state_t mstate = MAC_IDLE);

/* Length of the run of zeroes seen during MAC_IDLE */
bl_symbol_bss(static uint8_t idle_zeros);

/* Contents of the current sync byte */
bl_symbol_bss(static uint8_t mac_sync[4]);

/* Number of sync bytes received so far */
bl_symbol_bss(static uint8_t sync_count);

/* Expected length of this packet */
bl_symbol(static uint32_t pkt_len);

/* Number of bytes we've read so far */
bl_symbol(static uint32_t pkt_read);

// put_bit with a MAC layer on it
void putBitMac(int bit) {

  switch(mstate) {
  case MAC_IDLE:
    // Search until at least 32 zeros are found.
    // The next transition /might/ be sync.
    if (idle_zeros > 32) {
      if (bit != 0) {
        mstate = MAC_SYNC;
        g_bitpos = 6;
        g_curbyte = 0x80;
        sync_count = 0;
      }
      else {
        if (idle_zeros < 255)
          idle_zeros++;
      }
    }
    else {
      if (bit != 0)
        idle_zeros = 0;
      else
        idle_zeros++;
    }
    break;
    
  case MAC_SYNC:
    // acculumate two bytes worth of temp data, then check to see if valid sync
    g_curbyte >>= 1;
    g_bitpos--;
    if (bit)
      g_curbyte |= 0x80;
    
    /* We haven't yet read 8 bits */
    if (g_bitpos)
      break;

    /* 8 bits have been read.  Process the resulting byte. */

    /* Optimization: check to see if we just read an idle value. */
    if (g_curbyte == 0x00) {
      
      /* False noise trigger, go back to idle. */
      mstate = MAC_IDLE;
      idle_zeros = 8; /* We just saw 8 zeros, so count those */
      break;
    }

    /* Tally up the sync characters, make sure the sync matches. */

    mac_sync[sync_count++] = g_curbyte;
    if (sync_count >= 3) {
      /* Test for sync sequence. It's one byte less than the # of
       * leading zeros, to allow for the idle escape trick above to work
       * in case of zero-biased noise.
       */
      if ((mac_sync[0] == 0xAA)
       && (mac_sync[1] == 0x55)
       && (mac_sync[2] == 0x42)) {
        // found the sync sequence, proceed to packet state
        osalDbgAssert(pktReady == 0, "Packet buffer full flag still set "
                                     "while new packet incoming\r\n");
        mstate = MAC_PACKET;
        pkt_ptr = 0;
        pkt_len = 0;
        pkt_read = 0;
      }
      else {
        mstate = MAC_IDLE;
        idle_zeros = 0;
      }
    }

    /* Move on to the next bit */
    g_bitpos = 8;
    g_curbyte = 0;
    break;
    
  case MAC_PACKET:

    /* If we haven't figured out the length, but we've read the entire
     * header, figure out the length.  Or exit the loop if it's not a
     * valid packet.
     */
    if (!pkt_len && pkt_read > sizeof(demod_pkt_header_t)) {

      /* If the version number doesn't match, abandon ship. */
      if (pkt.header.version != PKT_VER)
        goto make_idle;

      if (pkt.header.type == PKTTYPE_CTRL)
        pkt_len = CTRL_LEN;
      else if (pkt.header.type == PKTTYPE_DATA)
        pkt_len = DATA_LEN;
      else
        /* Unrecognized packet type */
        goto make_idle;
    }

    /* Load on the next bit. */
    g_curbyte >>= 1;
    g_bitpos--;
    if (bit)
      g_curbyte |= 0x80;

    /* If we've finished this bit, add it to the packet. */
    if (g_bitpos == 0) {
      ((uint8_t *)&pkt)[pkt_read++] = g_curbyte;
      g_bitpos = 8;
      g_curbyte = 0;
    }

    /* If we've finished reading the packet, indicate it's ready */
    if (pkt_len && pkt_read >= pkt_len) {
      pktReady = 1; // flag that the packet is ready
      goto make_idle;
      break;
    }

    break;

  default:
    break;
  }

  return;

make_idle:
  mstate = MAC_IDLE;
  idle_zeros = 0;
}
