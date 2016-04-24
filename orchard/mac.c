#include "ch.h"
#include "mac.h"

// mac state is just local to mac
typedef enum states {
  MAC_IDLE = 0,
  MAC_SYNC,
  MAC_PACKET
} mac_state;

volatile uint16_t pktPtr = 0;
uint8_t  pktBuf[PKT_LEN];

static int g_bitpos = 9;
static unsigned char g_curbyte = 0;

static mac_state mstate = MAC_IDLE;
static uint8_t idle_zeros = 0;
static uint8_t mac_sync[4] = {0,0,0,0};
static uint8_t wordcnt = 0;

// put_bit with a MAC layer on it
void putBitMac(int bit) {
  
  switch(mstate) {
  case MAC_IDLE:
    // search until at least 32 zeros are found, then next transition "might" be sync
    if( idle_zeros > 32 ) {
      if( bit != 0 ) {
	mstate = MAC_SYNC;
	g_bitpos = 6;
	g_curbyte = 0x80;
	wordcnt = 0;
      } else {
	if(idle_zeros < 255)
	  idle_zeros++;
      }
    } else {
      if( bit != 0 )
	idle_zeros = 0;
      else
	idle_zeros++;
    }
    break;
    
  case MAC_SYNC:
    // acculumate two bytes worth of temp data, then check to see if valid sync
    g_curbyte >>= 1;
    g_bitpos--;
    if( bit )
      g_curbyte |= 0x80;
    
    if( g_bitpos == 0 ) {
      if( g_curbyte == 0x00 ) {  // false noise trigger, go back to idle
	mstate = MAC_IDLE;
	idle_zeros = 8; // we just saw 8 zeros, so count those
	break;
      }
      // else, tally up the sync characters
      mac_sync[wordcnt++] = g_curbyte;
      g_bitpos = 8;
      g_curbyte = 0;
      if( wordcnt == 3 ) {
	// test for sync sequence. It's one byte less than the # of leading zeros
	// to allow for the idle escape trick above to work in case of zero-biased noise
	if( (mac_sync[0] == 0xAA) && (mac_sync[1] == 0x55) && (mac_sync[2] = 0x42)) {
	  // found the sync sequence, proceed to packet state
	  mstate = MAC_PACKET;
	  pktPtr = 0;
	} else {
	  mstate = MAC_IDLE;
	  idle_zeros = 0;
	}
      }
    }
    break;
    
  case MAC_PACKET:
    if( pktPtr < PKT_LEN ) {
      g_curbyte >>= 1;
      g_bitpos--;
      if( bit )
	g_curbyte |= 0x80;
      
      if(g_bitpos == 0) {
	pktBuf[pktPtr++] = g_curbyte;
	g_bitpos = 8;
	g_curbyte = 0;
      }
    } else {
      mstate = MAC_IDLE;
      idle_zeros = 0;
    }
    break;

  default:
    break;
  }
}

