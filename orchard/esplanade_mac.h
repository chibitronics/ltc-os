#ifndef __ORCHARD_MAC__
#define __ORCHARD_MAC__

#define PAYLOAD_LEN  256

#define MURMUR_SEED_BLOCK  (0xdeadbeef)
#define MURMUR_SEED_TOTAL  (0x32d0babe)

typedef struct demod_ph {

  /* Protocol version */
  uint8_t version;

  /* Packet type */
  uint8_t type;
} __attribute__((packed)) demod_pkt_header_t;

typedef struct demod_dp {

  demod_pkt_header_t header;

  /* Block number (offset is block * PAYLOAD_LEN) */
  uint16_t  block;

  /* Payload contents */
  uint8_t   payload[PAYLOAD_LEN];

  /* Hash of this data packet */
  uint32_t  hash;
} __attribute__((packed)) demod_pkt_data_t;

typedef struct demod_cp {

  demod_pkt_header_t header;

  /* Padding */
  uint16_t reserved;

  /* Total length of program in bytes (# blocks = ceil(length / blocksize) */
  uint32_t length;

  /* Lightweight data integrity hash, computed across "length"
   * of program given above.
   */
  uint32_t fullhash;

  /* UID code for identifying a program uniquely, and globally */
  uint8_t guid[16];

  /* Hash check of /this/ packet */
  uint32_t hash;
} __attribute__((packed)) demod_pkt_ctrl_t;

typedef union demod_packet {
  demod_pkt_header_t header;
  demod_pkt_ctrl_t ctrl_pkt;
  demod_pkt_data_t data_pkt;
} __attribute__((packed)) demod_pkt_t;

// size of the largest packet we could receive
#define DATA_LEN  (sizeof(demod_pkt_data_t))
#define CTRL_LEN  (sizeof(demod_pkt_ctrl_t))

// bit 7 defines packet type on the version code (first byte received)
#define PKTTYPE_CTRL  0x01
#define PKTTYPE_DATA  0x02

#define PKT_VER_1 0x01
#define PKT_VER_2 0x02 /* Improved baud striping */

extern volatile uint16_t pktPtr;
extern demod_pkt_t pkt;
extern uint8_t  pktReady;

void putBitMac(int bit);

#endif
