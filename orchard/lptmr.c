
#include "hal.h"
#include "kl02.h"
#include "memio.h"
#include "lptmr.h"

#define KINETIS_LPTMR0_HANDLER  VectorB0

static uint8_t lptmr_enabled = 0;
static uint8_t lptmr_running = 0;
static uint8_t lptmr_pad;
static uint32_t lptmr_rate = 0;
static ioportid_t lptmr_port;

int (*lptmrFastISR)(void);
void (*lptmrISR)(void);

static void calibrate_irc(void) {

  const uint32_t expected_count = 21 * ((KINETIS_SYSCLK_MAX / 4) / 4000000) * 256;
  writeb((expected_count >> 8) & 0xff, MCG_ATCVH);
  writeb((expected_count >> 0) & 0xff, MCG_ATCVL);

  /* Select 4 MHz IRC */
  writeb(MCG_SC_ATMS, MCG_SC);
  writeb(MCG_SC_ATMS | MCG_SC_ATME, MCG_SC);

  while (readb(MCG_SC) & MCG_SC_ATME)
    ;
}

#define RATE 2000000 /* Sourced from 4 MHz IRC via a /2 divider */
void enableLptmr(void) {

  if (lptmr_enabled)
    return;

  /* Ungate clock */
  SIM->SCGC5 |= SIM_SCGC5_LPTMR;

  /* Reset the LPTMR block */
  writel(0, LPTMR0_CSR);

  calibrate_irc();

  /* Select fast internal reference clock (4 MHz) */
  writeb(readb(MCG_C2) | MCG_C2_IRCS, MCG_C2);

  /* Enable MCGIRCLK */
  writeb(readb(MCG_C1) | MCG_C1_IRCLKEN, MCG_C1);

  /* Use the 32 kHz crystal in bypass mode */
  //writel(LPTMR_PSR_PBYP | LPTMR_PSR_PCS_ERCLK32K, LPTMR0_PSR);

  /* Use MCGIRCLK in bypass mode */
  writel(LPTMR_PSR_PBYP | LPTMR_PSR_PCS_MCGIRCLK, LPTMR0_PSR);

  /* Use OSCERCLK in bypass mode */
  //writel(LPTMR_PSR_PBYP | LPTMR_PSR_PCS_OSCERCLK, LPTMR0_PSR);

  /* Use MCGIRCLK with a divide-by-two */
  //writel((0 <<3) | LPTMR_PSR_PCS_MCGIRCLK, LPTMR0_PSR);

  NVIC_EnableIRQ(28);
  writel(LPTMR_CSR_TIE, LPTMR0_CSR);

  lptmr_enabled = 1;
}

void startLptmr(ioportid_t port, uint8_t pad, uint32_t rate_hz) {

  /* Avoid div-by-0 */
  if (!rate_hz)
    return;

  lptmr_rate = RATE/rate_hz;

  enableLptmr();

  /* If the timer hasn't been enabled, update the period field and start it.
   * NOTE: If the timer is already going, we don't change the port/pad,
   * and instead use the old pad.
   */
  if (!lptmr_running) {
    lptmr_running = 1;
    lptmr_port = port;
    lptmr_pad = pad;
    writel(lptmr_rate, LPTMR0_CMR);
    writel(LPTMR_CSR_TIE | LPTMR_CSR_TEN, LPTMR0_CSR);
  }
}

void stopLptmr(void) {

  if (!lptmr_running)
    return;

  lptmr_running = 0;
  writel(0, LPTMR0_CSR);
}

OSAL_IRQ_HANDLER(KINETIS_LPTMR0_HANDLER) {
  if (lptmrFastISR) {
    if (!lptmrFastISR())
      return;
  }

  OSAL_IRQ_PROLOGUE();

  if (lptmrISR) {
    lptmrISR();
  }
  else {
    /* Ignore spurrious interrupts that happen for some reason */
    if (!lptmr_running)
      return;

    palTogglePad(lptmr_port, lptmr_pad);

    /* Update the counter with the new value (if it's changed).*/
    writel(lptmr_rate, LPTMR0_CMR);

    /* Clear the TCF bit, which lets the timer continue.*/
    writel(LPTMR_CSR_TCF | LPTMR_CSR_TIE | LPTMR_CSR_TEN, LPTMR0_CSR);
  }

  OSAL_IRQ_EPILOGUE();
}
