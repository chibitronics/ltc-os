/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"

#include "printf.h"

#include "orchard.h"

#include "esplanade_analog.h"
#include "esplanade_demod.h"
#include "esplanade_mac.h"
#include "esplanade_updater.h"
#include "esplanade_app.h"

#include "murmur3.h"

#include "kl02.h"

#define BOOT_AFTER_DELAY FALSE

#include <string.h>

static const I2CConfig i2c_config = {
  100000
};

void *stream;

#define DEBUG_STREAMING  0
#define OSCOPE_PROFILING 0
uint8_t screenpos = 0;

// global flag, careful of sync issues when multi-threaded...
volatile uint8_t dataReadyFlag = 0;
volatile adcsample_t *bufloc;
size_t buf_n;

static const SerialConfig serialConfig = {
  9600,
};

static const ADCConfig adccfg1 = {
  /* Perform initial calibration */
  true
};

static void phy_demodulate(void) {
  int frames;

#if OSCOPE_PROFILING // pulse a gpio to easily measure CPU load of demodulation
  GPIOB->PSOR |= (1 << 6);   // sets to high
  GPIOB->PCOR |= (1 << 6);   // clears to low

  // this is happening once every 1.748ms with NB_FRAMES = 16, NB_SAMPLES = 8
  // computed about 0.0413ms -> 41.3us per call overhead for OS required ~2.5% overhead
#endif

  for (frames = 0; frames < NB_FRAMES; frames++)
    // putBitMac is callback to MAC layer
    FSKdemod(dm_buf + (frames * NB_SAMPLES), NB_SAMPLES, putBitMac);

  dataReadyFlag = 0;
}

static void boot_if_timed_out_and_valid(void) {
#if (BOOT_AFTER_DELAY == TRUE)
  static int counter;
  if (! (++counter & 0xfffff)) {
    if (appIsValid()) {
      printf("Boot\r\n");
      chThdExit(0);
    }
  }
#endif
}

void demod_loop(void) {
  uint32_t i;

  NVIC_DisableIRQ(PendSV_IRQn);
  NVIC_DisableIRQ(SysTick_IRQn);
  // infinite loops, prevents other system items from starting
  while (TRUE) {
    while (!pktReady) {
      if (dataReadyFlag) {
        // copy from the double-buffer into a demodulation buffer
        for (i = 0 ; i < buf_n; i++) {
          dm_buf[i] = (int16_t) (((int16_t) bufloc[i]) - 2048);
        }
        // call handler, which includes the demodulation routine
        phy_demodulate();
      }

      boot_if_timed_out_and_valid();
    }

    // unstripe the transition xor's used to keep baud sync
    if (pkt.header.type == PKTTYPE_DATA) {
      /* We don't xor the header or the ending hash, but xor everything else */
      for (i = sizeof(pkt.header);
           i < sizeof(pkt.data_pkt) - sizeof(pkt.data_pkt.hash);
           i++) {
        if ((i % 16) == 7)
          ((uint8_t *)&pkt)[i] ^= 0x55;
        else if ((i % 16) == 15)
          ((uint8_t *)&pkt)[i] ^= 0xAA;
      }
    }

#define RAWDATA_CHECK 0
#if RAWDATA_CHECK
    uint32_t hash;
    uint32_t txhash;
    uint16_t pkt_len;
    // replace the code in this #if bracket with the storage flashing code
    if (pkt.header.type == PKTTYPE_DATA) {
      pkt_len = DATA_LEN;
      tfp_printf( "\n\r data packet:" );
    }
    else {
      tfp_printf( "\n\r control packet:" );
      pkt_len = CTRL_LEN;
    }

    for (i = 0; i < 16; i++) { // abridged dump
      if (i % 32 == 0) {
        tfp_printf( "\n\r" );
      }
      tfp_printf( "%02x", ((uint8_t *)&pkt)[i] /* isprint(pktBuf[i]) ? pktBuf[i] : '.'*/ );
    }

    // check hash
    MurmurHash3_x86_32(&pkt,
                       pkt_len - 4 /* packet minus hash */,
                       MURMUR_SEED_BLOCK,
                       &hash);

    /* The hash is always the last element of the packet, regardless of type. */
    txhash = ((uint32_t *)(((void *)&pkt) + pkt_len))[-1];

    tfp_printf(" tx: %08x rx: %08x\n\r", txhash, hash);
    if (txhash != hash)
      tfp_printf( " fail\n\r" );
    else
      tfp_printf( " pass\n\r" );

    pktReady = 0; // we've extracted packet data, so clear the buffer flag
#else

    updaterPacketProcess(&pkt);
#endif
  }
}

static size_t heap_size(void) {
  extern uint32_t __heap_base__;
  extern uint32_t __heap_end__;
  return (size_t) (&__heap_end__ - &__heap_base__);
}

/* Initialize the bootloader setup */
int blbss_len;
static void blcrt_init(void) {
  /* Variables defined by the linker */
  extern uint32_t _textbldata_start;
  extern uint32_t _bldata_start;
  extern uint32_t _bldata_end;
  extern uint32_t _blbss_start;
  extern uint32_t _blbss_end;

  blbss_len = ((uint32_t)&_blbss_end) - ((uint32_t)&_blbss_start);
  memset(&_blbss_start, 0, blbss_len);
  memcpy(&_bldata_start, &_textbldata_start, ((uint32_t)&_bldata_end) - ((uint32_t)&_bldata_start));
}

/*
 * "main" thread, separate from idle thread
 */
#include "esplanade_os.h"
#if defined(_CHIBIOS_RT_) /* Put this in bootloader area for Rt */
bl_symbol_bss(
#endif
    static THD_WORKING_AREA(waDemodThread, 256)
#if defined(_CHIBIOS_RT_)
    );
#endif
static THD_FUNCTION(demod_thread, arg) {
  (void)arg;

  GPIOB->PSOR |= (1 << 6);   // red off
  GPIOB->PCOR |= (1 << 7);   // green on
  GPIOB->PSOR |= (1 << 10);  // blue off

  // init the serial interface
  sdStart(&SD1, &serialConfig);
  stream = stream_driver;

  printf("%d free\r\n", heap_size());
  printf("Copyright (c) 2016 Chibitronics PTE LTD\r\n");

  i2cStart(i2cDriver, &i2c_config);
  adcStart(&ADCD1, &adccfg1);
  analogStart();
  demodInit();

  /*
    clock rate: 0.020833us/clock, 13.3us/sample @ 75kHz
    jitter notes: 6.8us jitter on 1st cycle; out to 11.7us on last cycle
    each frame of 8 samples (call to FSKdemod() takes ~56.3us to process, with a ~2.5us gap between calls to FSKdemod()
    a total of 32 frames is taking:
       1.867-1.878ms (1.872ms mean) to process (random noise),
       1.859-1.866ms (1.863ms mean) to process (0 tone),
       1.862-1.868ms (1.865ms mean) to process (1 tone),
       ** jitter seems to be data-dependent differences in code path length
    every 3.480ms +/- 2us -> 261 samples.
      +/-2us jitter due to ~when we catch 13.3us/sample edge vs system state (within synchronizing tolerance)
    **should be 3.413ms -> 256 samples, 67 microseconds are "extra" -> 5.025 -> 5 samples per 256 samples

    hypotheses:
      - actual effective sample rate is not 75kHz, it's 76.464kHz
        * measured rate = 13.34us(13.25-13.43us jitter spread) => 74.962kHz (within 500ppm correct)
        ** however! every 3.481ms (287Hz) we have an extra-wide gap at 87.76us (4.21k cycles), with a fast
           second sample time of ~5.329us - 5.607us later (e.g., the natural next point to grab a sample).
        ** this happens in the middle of the IRQ handler. the gap is actually from 87.07us-87.56us long.
        ** fwiw the actual ADC handler completes in 752ns fairly deterministically
      - we're deterministically missing 5 interrupts every cycle
      - there's a coding bug causing us to mis-count # samples

    other notes:
      - adding print's during runtime adds jitter to the processing time,
        but the processing start is deterministic to within 1.8us
      - processing start determinism is improved by putting constant data in
      - we've counted 32 frames being processed during the processing times
   */
  //(gdb) x 0xe000e180   // shows the interrupts that are enabled
  //0xe000e180:0x00009000
  // x/32x 0xe000e400
  NVIC_SetPriority(ADC0_IRQn, 0);
  NVIC_SetPriority(UART0_IRQn, 3);

  // for environment consitency sake, we don't check reset/hold pin status until here
  // not sure what of the above init is assumed by boot to app, so turn it all on
  // even if we just turn it all off later.

  PROG_STATR_ON;
  PROG_STATG_ON;
  chThdSleepMilliseconds(500);  // wait for user to let go of the button
  // reset_pulse is nom 2ms wide, so this is good margin
  PROG_STATR_OFF; // toggle LEDs to indicate we're now out of init
  PROG_STATG_OFF;

  printf("LtC build %s\r\n", gitversion); // emit build number later, so it shows up clearly on dataviewer

  if( RESET_LEVEL ) {
    // user did not hold down the reset button, check if app is valid and run it
    if(appIsValid()) {
      PROG_STATG_ON;  // indicate we're running user code
      PROG_STATR_OFF; 
      chThdExit(0);
    }
  }

  PROG_STATG_OFF;  // indicate we're waiting for code upload
  PROG_STATR_ON; 
  
  analogUpdateMic();  // starts mic sampling loop (interrupt-driven and automatic)
  demod_loop();
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
#if defined(_CHIBIOS_NIL_)
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waDemodThread, "demod", demod_thread, NULL)
THD_TABLE_END
#endif /* defined(_CHIBIOS_NIL_) */

/*
 * Application entry point.
 */
int main(void)
{
  blcrt_init();
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */

  halInit();
  chSysInit();
  
  /* Clear the onboard LED to prevent us from blinding people. */
  {
    extern void ledUpdate(uint32_t num_leds, void *pixels, uint32_t mask,
                          uint32_t set_addr, uint32_t clr_addr);
    uint32_t pixel = 0;
    ledUpdate(1, &pixel, (1 << 6), FGPIOA_PSOR, FGPIOA_PCOR);
  }
  
  // these lights both start "on"; differentiates crashes in boot from a bad power cable
  PROG_STATR_ON;
  PROG_STATG_ON;

#if defined(_CHIBIOS_RT_)
  volatile thread_t *demod_thread_p;
  demod_thread_p = chThdCreateStatic(waDemodThread, sizeof(waDemodThread),
                                     HIGHPRIO, demod_thread, NULL);
  /* Wait for thread to exit */
  while (demod_thread_p->p_state != CH_STATE_FINAL)
    ;
  demod_thread_p = NULL;
  NVIC_EnableIRQ(PendSV_IRQn);
  NVIC_EnableIRQ(SysTick_IRQn);
  analogStop();
  chBootToApp();
#endif

  while(1)  /// this is now the "idle" thread
    ;

}
