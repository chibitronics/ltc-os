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

#include "ch.h"
#include "hal.h"
#include "i2c.h"
#include "pal.h"
#include "adc.h"

#include "shell.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "orchard-events.h"

#include "accel.h"
#include "analog.h"
#include "demod.h"
#include "mac.h"

#include "kl02x.h"

#include "murmur3.h"

#define DEBUG_STREAMING  0
#define OSCOPE_PROFILING 0
uint8_t screenpos = 0;

struct evt_table orchard_app_events;
event_source_t accel_event;
event_source_t accel_test_event;
event_source_t ta_update_event;
event_source_t adc_celcius_event;
event_source_t adc_mic_event;

char xp, yp, zp;

volatile uint8_t dataReadyFlag = 0; // global flag, careful of sync issues when multi-threaded...
volatile adcsample_t *bufloc;
size_t buf_n;

static const I2CConfig i2c_config = {
  100000
};

static const ADCConfig adccfg1 = {
  /* Perform initial calibration */
  true
};

static void accel_cb(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;

  chSysLockFromISR();
  chEvtBroadcastI(&accel_event);
  chSysUnlockFromISR();
}

static const EXTConfig ext_config = {
  {
    {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, accel_cb, PORTA, 10},
  }
};

static void adc_mic_handler(eventid_t id) {
  (void) id;
  // for now, just a placeholder
  return;
}

static void phy_demodulate(void) {
  int frames;

#if OSCOPE_PROFILING // pulse a gpio to easily measure CPU load of demodulation
  palWritePad(GPIOB, 6, PAL_HIGH); 
  palWritePad(GPIOB, 6, PAL_LOW);
  // this is happening once every 1.748ms with NB_FRAMES = 16, NB_SAMPLES = 8
  // computed about 0.0413ms -> 41.3us per call overhead for OS required ~2.5% overhead
#endif
  // demodulation handler based on microphone data coming in
  for( frames = 0; frames < NB_FRAMES; frames++ ) {
    FSKdemod(dm_buf + (frames * NB_SAMPLES), NB_SAMPLES, putBitMac); // putBitMac is callback to MAC layer
  }
  dataReadyFlag = 0;
}

static void adc_celcius_handler(eventid_t id) {
  (void) id;
  chprintf(stream, "celcius: %d\n\r", analogReadTemperature());
}

static void accel_pulse_handler(eventid_t id) {
  (void) id;

  if(pulse_axis & PULSE_AXIS_X) {
    //chprintf(stream, "x");
    xp = 'x';
  }
  if( pulse_axis & PULSE_AXIS_Y) {
    //chprintf(stream, "y");
    yp = 'y';
  }
  if( pulse_axis & PULSE_AXIS_Z) {
    //chprintf(stream, "z");
    zp = 'z';
  }
  chEvtBroadcast(&ta_update_event);
}

static void update_handler(eventid_t id) {
  (void) id;

}

void init_update_events(void) {
  chEvtObjectInit(&ta_update_event);
  evtTableHook(orchard_app_events, ta_update_event, update_handler);
}

void demod_test(void) {
  uint32_t i;
  uint32_t hash;
  uint32_t txhash;

  // stop systick interrupts
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
  nvicDisableVector(HANDLER_SYSTICK);
  
  // infinite loops, prevents other system items from starting
  while(TRUE) {
    pktPtr = 0;
    while( pktPtr < PKT_LEN ) {
      if( dataReadyFlag ) {
	// copy from the double-buffer into a demodulation buffer
	for( i = 0 ; i < buf_n; i++ ) { 
	  dm_buf[i] = (int16_t) (((int16_t) bufloc[i]) - 2048);
	}
	// call handler, which includes the demodulation routine
	phy_demodulate();
      }
    }

#if !DEBUG_STREAMING
    // replace the code in this #if bracket with the storage flashing code
    for( i = 0; i < PKT_LEN; i++ ) {
      if( i % 16 == 0 )
	chprintf(stream, "\n\r" );
      chprintf(stream, "%02x", pktBuf[i] /* isprint(pktBuf[i]) ? pktBuf[i] : '.'*/ );
      //      chprintf(stream, "%c", isprint(pktBuf[i]) ? pktBuf[i] : '.' );
    }

    // check hash
    MurmurHash3_x86_32(pktBuf, PKT_LEN - 4 /* subtract hash itself from hash */, 0xdeadbeef, &hash);
    //    chprintf(stream, " hash: %02x%02x%02x%02x\n\r",
    //	     hash & 0xff, (hash >> 8) & 0xff, (hash >> 16) & 0xff, (hash >> 24) & 0xff);

    txhash = (pktBuf[PKT_LEN-4] & 0xFF) | (pktBuf[PKT_LEN-3] & 0xff) << 8 |
      (pktBuf[PKT_LEN-2] & 0xFF) << 16 | (pktBuf[PKT_LEN-1] & 0xff) << 24;

    chprintf(stream, " txhash: %08x rxhash: %08x\n\r", txhash, hash);
    if( txhash != hash ) {
      chprintf(stream, " hash fail\n\r" );
    } else {
      chprintf(stream, " hash pass\n\r" );
    }
#endif
  }
  
}

/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  //i2cStart(i2cDriver, &i2c_config);
  adcStart(&ADCD1, &adccfg1);
  analogStart();
  
  demodInit();

  orchardShellInit();

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
#if DEMOD_DEBUG
  //(gdb) x 0xe000e180   // shows the interrupts that are enabled
  //0xe000e180:0x00009000
  // x/32x 0xe000e400
  NVIC_SetPriority(ADC0_IRQn, 0);
  NVIC_SetPriority(UART0_IRQn, 3);

  while( !(((volatile SysTick_Type *)SysTick)->CTRL & SysTick_CTRL_COUNTFLAG_Msk) )
    ;
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  
  NVIC_DisableIRQ(PendSV_IRQn);
  NVIC_DisableIRQ(SysTick_IRQn);
  //x/2x 0xe000ed1c
  NVIC_SetPriority(SVCall_IRQn, 3);
  NVIC_SetPriority(PendSV_IRQn, 3);
  NVIC_SetPriority(SysTick_IRQn, 3);
  NVIC_DisableIRQ(PendSV_IRQn);
  NVIC_DisableIRQ(SysTick_IRQn);
  
  analogUpdateMic();
  demod_test();
#endif

  // the rest of this stuff we don't use -- why? because we aren't thread safe yet
  // either eliminate this code, or figure out how to back threads into the system without
  // impacting reliable demodulation
  
  chprintf(stream, "\r\n\r\nOrchard shell.  Based on build %s\r\n", gitversion);

  orchardShellRestart();

  evtTableInit(orchard_app_events, 9);

  init_update_events();

  chEvtObjectInit(&adc_celcius_event);
  chEvtObjectInit(&adc_mic_event);
  evtTableHook(orchard_app_events, adc_celcius_event, adc_celcius_handler);
  evtTableHook(orchard_app_events, adc_mic_event, adc_mic_handler);
  
  //  chEvtObjectInit(&accel_event);
  //  evtTableHook(orchard_app_events, accel_event, accel_irq);
  //  accelStart(i2cDriver);

  //  chEvtObjectInit(&accel_test_event);
  //  evtTableHook(orchard_app_events, accel_test_event, accel_test);

  //  evtTableHook(orchard_app_events, accel_pulse, accel_pulse_handler);
  xp = ' '; yp = ' '; zp = ' ';
  
  extStart(&EXTD1, &ext_config); // enables interrupts on gpios

  // high is off
  palWritePad(GPIOB, 6, PAL_HIGH);  // red
  palWritePad(GPIOB, 7, PAL_LOW);  // green
  palWritePad(GPIOB, 10, PAL_HIGH);  // blue

  while (TRUE) {
    chEvtDispatch(evtHandlers(orchard_app_events), chEvtWaitOne(ALL_EVENTS));
  }

}

