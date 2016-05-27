#include <stdint.h>

#include "hal.h"

#include "Arduino.h"

#define PCR_IRQC_LOGIC_ZERO         0x8
#define PCR_IRQC_RISING_EDGE        0x9
#define PCR_IRQC_FALLING_EDGE       0xA
#define PCR_IRQC_EITHER_EDGE        0xB
#define PCR_IRQC_LOGIC_ONE          0xC

struct USBPHY;
struct USBMAC;
struct USBLink;

void usbAttach(struct USBPHY *phy) {
  (void)phy;
}

void usbDetach(struct USBPHY *phy) {
  (void)phy;
}

int usbSetup(struct USBPHY *phy, struct USBMAC *mac, struct USBLink *link) {

  (void)phy;
  (void)mac;
  (void)link;

  return 0;
#if 0
  ioportid_t port;
  uint8_t pad;
  int i;
  uint32_t pcr;

  if (!phy || !mac || !link)
    return -1;

  /* Depriorotize timer IRQs, prioritize our IRQ */
  NVIC_SetPriority(SVCall_IRQn, 1);
  NVIC_SetPriority(PendSV_IRQn, 1);
  NVIC_SetPriority(SysTick_IRQn, 1);
  for (i = 0; i < CORTEX_NUM_VECTORS; i++)
    if (NVIC_GetPriority(i) == 0)
      NVIC_SetPriority(i, 1);

  usbMacInit(mac, link);
  usbPhyInit(phy, mac);

  /* Mux the pins as GPIO inputs */
  pinToPort(UART_RX, &port, &pad);
  palSetPadMode(port, pad, PAL_MODE_INPUT);

  pinToPort(UART_TX, &port, &pad);
  palSetPadMode(port, pad, PAL_MODE_INPUT);

  /* Enable the IRQ */
  pcr = PORTB->PCR[3];
  /* Clear all the IRQC bits */
  pcr &= ~PORTx_PCRn_IRQC_MASK;
  /* Set the required IRQC bits */
  pcr |= PORTx_PCRn_IRQC(PCR_IRQC_EITHER_EDGE);
  PORTB->PCR[3] = pcr;

  /* Enable the IRQ */
  pcr = PORTB->PCR[4];
  /* Clear all the IRQC bits */
  pcr &= ~PORTx_PCRn_IRQC_MASK;
  /* Set the required IRQC bits */
  pcr |= PORTx_PCRn_IRQC(PCR_IRQC_EITHER_EDGE);
  PORTB->PCR[4] = pcr;

  /* Enable the PORTB IRQ, with the highest possible priority.*/
  nvicEnableVector(PINB_IRQn, 0);
  NVIC_SetPriority(PINB_IRQn, 0);

  return 0;
#endif
}

int usbSend(struct USBMAC *mac, int epnum, const void *data, int count) {
  (void)mac;
  (void)epnum;
  (void)data;
  (void)count;
  return 0;
}

int usbReceive(struct USBMAC *mac, void (*callback)(const void *data, int count)) {
  (void)mac;
  (void)callback;
  return 0;
}
