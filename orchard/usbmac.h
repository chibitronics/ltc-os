#ifndef __USB_MAC_H__
#define __USB_MAC_H__

enum usb_pids {
  USB_PID_RESERVED = 0xf0,
  USB_PID_OUT = 0xe1,
  USB_PID_ACK = 0xd2,
  USB_PID_DATA0 = 0xc3,
  USB_PID_PING = 0xb4,
  USB_PID_SOF = 0xa5,
  USB_PID_NYET = 0x96,
  USB_PID_DATA2 = 0x87,
  USB_PID_SPLIT = 0x78,
  USB_PID_IN = 0x69,
  USB_PID_NAK = 0x5a,
  USB_PID_DATA1 = 0x4b,
  USB_PID_ERR = 0x3c,
  USB_PID_SETUP = 0x2d,
  USB_PID_STALL = 0x1e,
  USB_PID_MDATA = 0x0f,
};

enum usb_pids_rev {
  USB_DIP_SPLIT = 0x1e,
  USB_DIP_PING = 0x2d,
  USB_DIP_PRE = 0x3c,
  USB_DIP_ACK = 0x4b,
  USB_DIP_NAK = 0x5a,
  USB_DIP_NYET = 0x69,
  USB_DIP_STALL = 0x78,
  USB_DIP_OUT = 0x87,
  USB_DIP_IN = 0x96,
  USB_DIP_SOF = 0xa5,
  USB_DIP_SETUP = 0xb4,
  USB_DIP_DATA0 = 0xc3,
  USB_DIP_DATA1 = 0xd2,
  USB_DIP_DATA2 = 0xe1,
  USB_DIP_MDATA = 0xf0,
};

struct usb_packet {
  union {
    struct {
      uint8_t pid;
      uint8_t data[10]; /* Including CRC */
    };
    uint8_t raw_data[11];
  };
  uint8_t size; /* Not including pid (so may be 0) */
  /* Checksum omitted */
} __attribute__((packed, aligned(4)));

enum usb_mac_packet_type {
  packet_type_none,
  packet_type_setup,
  packet_type_in,
  packet_type_out,
};

struct usb_mac_setup_packet {
  uint8_t bmRequestType;
  uint8_t bRequest;
  union {
    uint16_t wValue;
    struct {
      uint8_t wValueH;
      uint8_t wValueL;
    };
  };
  uint16_t wIndex;
  uint16_t wLength;
} __attribute__((packed, aligned(4)));

struct USBLink;

struct USBMAC {
  struct USBPHY *phy;
  struct USBLink *link;
  int phy_internal_is_ready;

  union {
    uint8_t data_in[8];
    struct usb_mac_setup_packet data_setup;
  };

  const void *data_out;
  int32_t data_out_left;
  int32_t data_out_max;

#if defined(_CHIBIOS_RT_)
  thread_reference_t thread;
#endif

  struct usb_packet packet; /* Currently-queued packet */
  int packet_queued;    /* Whether a packet is queued */

  uint8_t data_buffer;  /* Whether we're sending DATA0 or DATA1 */
  uint8_t packet_type;  /* PACKET_SETUP, PACKET_IN, or PACKET_OUT */

  uint8_t address;      /* Our configured address */

  uint8_t tok_addr;     /* Last token's address */
  uint8_t tok_epnum;    /* Last token's endpoint */
} __attribute((packed, aligned(4)));

/* Process all packets sitting in the queue */
int usbMacProcess(struct USBMAC *mac,
                  const uint8_t packet[11],
                  uint32_t count);

/* Initialize the given USB MAC */
void usbMacInit(struct USBMAC *mac, struct USBLink *link);

/* Set the given MAC to use the given PHY */
void usbMacSetPhy(struct USBMAC *mac, struct USBPHY *phy);

/* Set the USB link to the provided device */
void usbMacSetLink(struct USBMAC *mac, struct USBLink *link);

/* Get the default, system-wide USB MAC */
struct USBMAC *usbMacDefault(void);

/* Get the PHY associated with this MAC */
struct USBPHY *usbMacPhy(struct USBMAC *mac);

/* Indicate that the transfer concluded successfully */
void usbMacTransferSuccess(struct USBMAC *mac);

int usbSendData(struct USBMAC *mac, int epnum, const void *data, int count);

#endif /* __USB_MAC_H__ */
