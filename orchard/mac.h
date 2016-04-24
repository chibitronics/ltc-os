#ifndef __ORCHARD_MAC__
#define __ORCHARD_MAC__

#define PAYLOAD_LEN  256

#define MURMUR_SEED_BLOCK  (0xdeadbeef)
#define MURMUR_SEED_TOTAL  (0x032dbabe)

typedef struct demod_dp {
  uint8_t  version;
  uint8_t  block[2];  // can't do uint16_t because of struct alignment issues in C
  uint8_t  payload[PAYLOAD_LEN];
  uint8_t hash[4];    // again, can't do uint32_t due to struct alignment issues
} demod_data_pkt;

typedef struct demod_cp {
  uint8_t version;
  uint8_t length[4];    // total length of program in bytes (# blocks = ceil(length / blocksize)
  uint8_t fullhash[4];  // lightweight data integrity hash, computed across "length" of program given above
  uint8_t guid[16];  // uid code for identifying a program uniquely, and globally
  uint8_t hash[4];   // the hash check of /this/ packet
} demod_ctrl_pkt;

#define PKT_LEN  (sizeof(demod_data_pkt))   // size of the largest packet we could receive
#define CTRL_LEN  (sizeof(demod_ctrl_pkt))

// bit 7 defines packet type on the version code (first byte received)
#define PKTTYPE_MASK  0x80
#define PKTTYPE_CTRL  0x80
#define PKTTYPE_DATA  0x00

// version codes for mac control & data packets
#define MAC_CTRL_VER  0x01
#define MAC_DATA_VER  0x01

extern volatile uint16_t pktPtr;
extern uint8_t  pktBuf[PKT_LEN];
extern uint8_t  pktReady;

void putBitMac(int bit);

#endif
