
#include "hal.h"
#include "kl02.h"
#include "memio.h"
#include "lptmr.h"

#define KINETIS_LPTMR0_HANDLER  VectorB0

static int lptmr_enabled = 0;
static int lptmr_running = 0;
static uint32_t lptmr_rate = 0;
static ioportid_t lptmr_port;
static uint8_t lptmr_pad;

#define RATE 234000000/88
static void lptmr_enable(void) {

  if (lptmr_enabled)
    return;

  /* Ungate clock */
  SIM->SCGC5 |= SIM_SCGC5_LPTMR;

  /* Reset the LPTMR block */
  writel(0, LPTMR0_CSR);

  /* Select fast internal reference clock */
  writeb(readb(MCG_C2) | (1 << 0), MCG_C2);

  /* Enable MCGIRCLK */
  writeb(readb(MCG_C1) | (1 << 1), MCG_C1);

  /* Use the 32 kHz crystal in bypass mode */
  //writel((1 << 2) | (2 << 0), LPTMR0_PSR);

  /* Use MCGIRCLK in bypass mode */
  writel((0 << 3) | (1 << 2) | (0 << 0), LPTMR0_PSR);

  NVIC_EnableIRQ(28);
  writel(LPTMR_CSR_TIE, LPTMR0_CSR);

  lptmr_enabled = 1;
}

void startLptmr(ioportid_t port, uint8_t pad, uint32_t rate_hz) {

  /* Avoid div-by-0 */
  if (!rate_hz)
    return;

  lptmr_rate = RATE/rate_hz;

  lptmr_enable();

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

  lptmr_running = 0;
  writel(0, LPTMR0_CSR);
}

OSAL_IRQ_HANDLER(KINETIS_LPTMR0_HANDLER) {
  OSAL_IRQ_PROLOGUE();

  /* Ignore spurrious interrupts that happen for some reason */
  if (!lptmr_running)
    return;

  palTogglePad(lptmr_port, lptmr_pad);

  /* Update the counter with the new value (if it's changed).*/
  writel(lptmr_rate, LPTMR0_CMR);

  /* Clear the TCF bit, which lets the timer continue.*/
  writel(LPTMR_CSR_TCF | LPTMR_CSR_TIE | LPTMR_CSR_TEN, LPTMR0_CSR);

  OSAL_IRQ_EPILOGUE();
}
