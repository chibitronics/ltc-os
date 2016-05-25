#include <string.h>
#include "hal.h"
#include "usbphy.h"
#include "usbmac.h"
#include "usblink.h"

static struct USBMAC default_mac;

static uint16_t crc16_add(uint16_t crc, uint8_t c, uint16_t poly)
{
  uint8_t  i;
  
  for (i = 0; i < 8; i++) {
    if ((crc ^ c) & 1) 
      crc = (crc >> 1) ^ poly;
    else 
      crc >>= 1;
    c >>= 1;
  }
  return crc;
}

static uint16_t crc16(const uint8_t *data, uint32_t count,
                      uint16_t init, uint32_t poly) {

  while (count--)
    init = crc16_add(init, *data++, poly);

  return init;
}

static void usb_mac_process_data(struct USBMAC *mac) {

  uint16_t crc;

  /* Don't allow us to re-prepare data */
  if (mac->packet_queued)
    return;

  /* If there's no data to send, then don't send any */
  if (!mac->data_out)
    return;

  /* If we've sent all of our data, then there's nothing else to send */
  if ((mac->data_out_left == 0) && (mac->data_out_max == 0)) {
    mac->data_out = NULL;
    return;
  }

  /* Pick the correct PID, DATA0 or DATA1 */
  if (mac->data_buffer++ & 1)
    mac->packet.pid = USB_PID_DATA1;
  else
    mac->packet.pid = USB_PID_DATA0;

  /* If there's no data, prepare a special NULL packet */
  if ((mac->data_out_left <= 0) || (mac->data_out_max <= 0)) {
    mac->data_out_left = 0;
    mac->data_out_max = 0;
    mac->data_out = NULL;
    mac->packet.data[0] = 0;
    mac->packet.data[1] = 0;
    usbPhyWritePrepare(mac->phy, &mac->packet, 2 + 1);
    return;
  }

  /* Keep the packet size to 8 bytes max */
  if (mac->data_out_left > 8)
    mac->packet.size = 8;
  else
    mac->packet.size = mac->data_out_left;

  /* Limit the amount of data transferred to data_out_max */
  if (mac->packet.size > mac->data_out_max)
    mac->packet.size = mac->data_out_max;

  /* Copy over data bytes */
  memcpy(mac->packet.data, mac->data_out, mac->packet.size);

  /* Calculate and copy the crc16 */
  crc = ~crc16(mac->packet.data, mac->packet.size, 0xffff, 0xa001);
  mac->packet.data[mac->packet.size++] = crc;
  mac->packet.data[mac->packet.size++] = crc >> 8;

  /* Prepare the packet, including the PID at the end */
  usbPhyWritePrepare(mac->phy, &mac->packet, mac->packet.size + 1);
  mac->packet_queued = 1;
}

void usbMacTransferSuccess(struct USBMAC *mac) {

  mac->packet_queued = 0;

  /* Reduce the amount of data left.
   * If the packet is divisible by 8, this will cause one more call
   * to this function with mac->data_out_left == 0.  This will send
   * a NULL packet, which indicates end-of-transfer.
   */
  if (mac->data_out) {
    mac->data_out_left -= 8;
    mac->data_out_max -= 8;
    mac->data_out += 8;
  }
  if ((mac->data_out_left < 0) || (mac->data_out_max < 0)) {
    mac->data_out_left = 0;
    mac->data_out_max = 0;
    mac->data_out = NULL;
  }
}

static int usb_mac_send_data(struct USBMAC *mac,
                             const void *data,
                             int count,
                             int max) {

  mac->data_out = data;
  mac->data_out_left = count;
  mac->data_out_max = max;

  return 0;
}

int usbSendData(struct USBMAC *mac, int epnum, const void *data, int count) {

  (void)epnum;
  int ret;

  ret = usb_mac_send_data(mac, data, count, count);
  if (ret)
    return ret;

  usb_mac_process_data(mac);

#if (CH_USE_RT == TRUE)
  (void) osalThreadSuspendS(&mac->thread);
#endif

  return 0;
}

static int usb_mac_process_setup_read(struct USBMAC *mac,
                                      const struct usb_mac_setup_packet *setup)
{
  const void *response = NULL;
  uint32_t len = 0;
  struct USBLink *link = mac->link;

  switch (setup->bmRequestType) {
  case 0x80:  /* Device-to-host, standard, read from device */
    switch (setup->bRequest) {

    /* GET_DESCRIPTOR */
    case 6:
      link->getDescriptor(link, setup, &response);
#if 0
      switch (setup->wValueL) {

      /* GET_DEVICE_DESCRIPTOR */
      case 1:
        len = link->getDeviceDescriptor(link, setup->wValueH, &response);
        break;

      /* GET_CONFIGURATION_DESCRIPTOR */
      case 2:
        len = link->getConfigurationDescriptor(link, setup->wValueH, &response);
        break;

      /* GET_STRING_DESCRIPTOR */
      case 3:
        len = link->getStringDescriptor(link, setup->wValueH, &response);
        break;

      }
#endif
      break;
    }
    break;
  case 0x81: /* Device-to-host, standard, read from interface */
    switch(setup->bRequest) {

    /* GET_CLASS_DESCRIPTOR */
    case 6:
      len = link->getClassDescriptor(link, setup, &response);
      break;
    }
    break;
  }

  usb_mac_send_data(mac, response, len, setup->wLength);
  return 0;
}

static int usb_mac_process_setup_write(struct USBMAC *mac,
                                       const struct usb_mac_setup_packet *setup)
{
  const void *response = (const void *)-1;
  uint8_t len = 0;
  uint8_t max = 1;
  struct USBLink *link = mac->link;

  switch (setup->bmRequestType) {
  case 0x00:  /* Device-to-host, standard, write to host */
    switch (setup->bRequest) {
    case 5: /* SET_ADDRESS */
      mac->address = setup->wValue;
      break;

    case 9: /* SET_CONFIGURATION */
      link->config_num = setup->wValue;
      break;
    }
    break;

  case 0x01: /* Device-to-host, standard, write to host */
    break;

  case 0x02: /* Device-to-host, standard, write to host */
    break;

  case 0x03: /* Device-to-host, standard, write to host */
    break;

  case 0x20: /* Device-to-host, class, write to host */
    break;

  case 0x21: /* Device-to-host, class, write to host */
    switch (setup->bRequest) {
    case 0x0a: /* Thingy? */
      /* This happens in Set Idle */
      break;
    }
    break;

  case 0x22: /* Device-to-host, class, write to host */
    break;

  case 0x23: /* Device-to-host, class, write to host */
    break;

  case 0x40: /* Device-to-host, vendor, write to host */
    break;

  case 0x41: /* Device-to-host, vendor, write to host */
    break;

  case 0x42: /* Device-to-host, vendor, write to host */
    break;

  case 0x43: /* Device-to-host, vendor, write to host */
    break;
  }

  /* We must always send a response packet.  If there's ever a time when
   * we shouldn't send a packet, simply "return" rather than "break" above.
   */
  usb_mac_send_data(mac, response, len, max);

  return 0;
}

static int usb_mac_process_setup(struct USBMAC *mac, const uint8_t packet[10]) {

  const struct usb_mac_setup_packet *setup;

  setup = (const struct usb_mac_setup_packet *)packet;

  if (setup->bmRequestType & 0x80)
    /* Device-to-Host */
    return usb_mac_process_setup_read(mac, setup);

  else
    return usb_mac_process_setup_write(mac, setup);

  return 0;
}

static inline void usb_mac_parse_token(struct USBMAC *mac,
                                       const uint8_t packet[2]) {

  mac->tok_addr = packet[0] >> 1;
  mac->tok_epnum = (packet[1] >> 5) | ((packet[0] << 5) & 1);
}

static void usb_mac_parse_data(struct USBMAC *mac,
                               const uint8_t packet[10],
                               uint32_t count) {
  (void)count;

  switch (mac->packet_type) {

  case packet_type_setup:
    usb_mac_process_setup(mac, packet);
    break;

  case packet_type_in:

    break;

  case packet_type_out:

    break;

  case packet_type_none:
    break;

  default:
    break;
  }
}

int usbMacProcess(struct USBMAC *mac,
                  const uint8_t packet[11],
                  uint32_t count) {

  switch(packet[0]) {
  case USB_PID_SETUP:
    mac->packet_type = packet_type_setup;
    usb_mac_parse_token(mac, packet + 1);
    break;

  case USB_PID_DATA0:
    mac->data_buffer = 1;
    usb_mac_parse_data(mac, packet + 1, count - 1);
    mac->packet_type = packet_type_none;
    break;

  case USB_PID_DATA1:
    mac->data_buffer = 0;
    usb_mac_parse_data(mac, packet + 1, count - 1);
    mac->packet_type = packet_type_none;
    break;

  case USB_PID_ACK:
#if (CH_USE_RT == TRUE)
    if (mac->thread && !mac->data_out) {
      mac->data_out_left = 0;
      mac->data_out_max = 0;
      mac->data_out = NULL;
      osalThreadResumeS(&mac->thread, MSG_OK);
    }
#endif
    break;

  default:
    break;
  }

  /* Pre-process any data that's remaining, so it can be sent out
   * in case an IN token comes.
   */
  usb_mac_process_data(mac);

  return 0;
}

void usbMacInit(struct USBMAC *mac, struct USBLink *link) {

  mac->link = link;
  mac->data_out = NULL;
  mac->data_out_left = 0;
  mac->data_out_max = 0;
}

void usbMacSetPhy(struct USBMAC *mac, struct USBPHY *phy) {

  mac->phy = phy;
}

void usbMacSetLink(struct USBMAC *mac, struct USBLink *link) {

  mac->link = link;
}

struct USBPHY *usbMacPhy(struct USBMAC *mac) {

  return mac->phy;
}

struct USBMAC *usbMacDefault(void) {

  return &default_mac;
}

void usbSendControl(void) {
  return;
}

void usbReceiveControl(void) {
  return;
}
