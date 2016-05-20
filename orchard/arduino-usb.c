#include <stdint.h>

#include "hal.h"

#include "usbphy.h"
#include "usbmac.h"
#include "usblink.h"

void usbAttach(struct USBPHY *phy) {
    usbPhyAttach(phy);
}

void usbDetach(struct USBPHY *phy) {
    usbPhyDetach(phy);
}

int usbSetup(struct USBPHY *phy, struct USBMAC *mac, struct USBLink *link) {
  if (!phy || !mac || !link)
    return -1;

  usbMacInit(mac, link);
  usbPhyInit(phy, mac);
  return 0;
}

int usbSend(struct USBMAC *mac, int epnum, const void *data, int count) {

  return usbSendData(mac, epnum, data, count);
}

int usbReceive(struct USBMAC *mac, void (*callback)(const void *data, int count)) {
  (void)mac;
  (void)callback;
#warning "Implement usbReceive()"

  return 0;
}