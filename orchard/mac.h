#ifndef __ORCHARD_MAC__
#define __ORCHARD_MAC__

#define PAYLOAD_LEN  256

typedef struct {
  uint8_t  version;
  uint8_t sector[2];  // can't do uint16_t because of struct alignment issues in C
  uint8_t  payload[PAYLOAD_LEN];
  uint8_t hash[4];    // again, can't do uint32_t due to struct alignment issues
} demod_data_pkt;

typedef struct {
  uint8_t version;
  uint8_t length[4];    // total length of program in bytes (# blocks = ceil(length / blocksize)
  uint8_t fullhash[4];  // lightweight data integrity hash, computed across "length" of program given above
  uint8_t  guid[16];  // uid code for identifying a program uniquely, and globally
} demod_ctrl_pkt;

#define PKT_LEN  (sizeof(demod_data_pkt))   // size of the largest packet we could receive

extern volatile uint16_t pktPtr;
extern uint8_t  pktBuf[PKT_LEN];

void putBitMac(int bit);

#endif
