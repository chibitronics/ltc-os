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

#include "kl02x.h"

#include "murmur3.h"

#define DEBUG_STREAMING  0
uint8_t screenpos = 0;

struct evt_table orchard_app_events;
event_source_t accel_event;
event_source_t accel_test_event;
event_source_t ta_update_event;
event_source_t adc_celcius_event;
event_source_t adc_mic_event;

char xp, yp, zp;

volatile uint8_t data_ready = 0;
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

static void print_mcu_info(void) {
  uint32_t sdid = SIM->SDID;
  const char *famid[] = {
    "KL0%d (low-end)",
    "KL1%d (basic)",
    "KL2%d (USB)",
    "KL3%d (Segment LCD)",
    "KL4%d (USB and Segment LCD)",
  };
  const uint8_t ram[] = {
    0,
    1,
    2,
    4,
    8,
    16,
    32,
    64,
  };

  const uint8_t pins[] = {
    16,
    24,
    32,
    36,
    48,
    64,
    80,
  };

  if (((sdid >> 20) & 15) != 1) {
    chprintf(stream, "Device is not Kinetis KL-series\r\n");
    return;
  }

  chprintf(stream, famid[(sdid >> 28) & 15], (sdid >> 24) & 15);
  chprintf(stream, " with %d kB of ram detected"
                   " (Rev: %04x  Die: %04x  Pins: %d).\r\n",
                   ram[(sdid >> 16) & 15],
                   (sdid >> 12) & 15,
                   (sdid >> 7) & 31,
                   pins[(sdid >> 0) & 15]);
}

volatile uint16_t log_ptr = 0;
uint8_t  log_buf[LOG_DEPTH];
static int g_bitpos = 9;
static unsigned char g_curbyte = 0;
static void put_bit(int bit)
{
  g_curbyte >>= 1;
  g_bitpos--;
  if( bit )
    g_curbyte |= 0x80;
  
  if( g_bitpos == 0 ) {
    if( log_ptr < LOG_DEPTH ) {
      log_buf[log_ptr++] = g_curbyte;
#if DEBUG_STREAMING
      if( (screenpos % 16 == 0) ) {
	chprintf(stream, "\n\r");
      }
      //      chprintf(stream, "%2x ", g_curbyte );
      chprintf(stream, "%c", isprint(g_curbyte) ? g_curbyte : '.' );
      screenpos++;
#endif
    }

    g_bitpos = 8;
    g_curbyte = 0;
  }
}

typedef enum states {
  MAC_IDLE = 0,
  MAC_SYNC,
  MAC_PACKET
} mac_state;
static mac_state mstate = MAC_IDLE;
static uint8_t idle_zeros = 0;
static uint8_t mac_temp[4] = {0,0,0,0};
static uint8_t wordcnt = 0;
// put_bit with a MAC layer on it
static void put_bit_mac(int bit) {
  
  switch(mstate) {
  case MAC_IDLE:
    // search until at least 32 zeros are found, then next transition "might" be sync
    if( idle_zeros > 32 ) {
      if( bit != 0 ) {
	mstate = MAC_SYNC;
	g_bitpos = 6;
	g_curbyte = 0x80;
	wordcnt = 0;
      } else {
	if(idle_zeros < 255)
	  idle_zeros++;
      }
    } else {
      if( bit != 0 )
	idle_zeros = 0;
      else
	idle_zeros++;
    }
    break;
    
  case MAC_SYNC:
    // acculumate two bytes worth of temp data, then check to see if valid sync
    g_curbyte >>= 1;
    g_bitpos--;
    if( bit )
      g_curbyte |= 0x80;
    
    if( g_bitpos == 0 ) {
      if( g_curbyte == 0x00 ) {  // false noise trigger, go back to idle
	mstate = MAC_IDLE;
	idle_zeros = 8; // we just saw 8 zeros, so count those
	break;
      }
      // else, tally up the sync characters
      mac_temp[wordcnt++] = g_curbyte;
      g_bitpos = 8;
      g_curbyte = 0;
      if( wordcnt == 3 ) {
	// test for sync sequence. It's one byte less than the # of leading zeros
	// to allow for the idle escape trick above to work in case of zero-biased noise
	if( (mac_temp[0] == 0xAA) && (mac_temp[1] == 0x55) && (mac_temp[2] = 0x42)) {
	  // found the sync sequence, proceed to packet state
	  mstate = MAC_PACKET;
	  log_ptr = 0;
	} else {
	  mstate = MAC_IDLE;
	  idle_zeros = 0;
	}
      }
    }
    break;
    
  case MAC_PACKET:
    if( log_ptr < LOG_DEPTH ) {
      g_curbyte >>= 1;
      g_bitpos--;
      if( bit )
	g_curbyte |= 0x80;
      
      if(g_bitpos == 0) {
	log_buf[log_ptr++] = g_curbyte;
	g_bitpos = 8;
	g_curbyte = 0;
      }
    } else {
      mstate = MAC_IDLE;
      idle_zeros = 0;
    }
    break;

  default:
    break;
  }
}


static void adc_mic_handler(eventid_t id) {
  (void) id;
  int frames;
#if 0
  //palWritePad(GPIOB, 6, PAL_HIGH); 
  //palWritePad(GPIOB, 6, PAL_LOW);
  // this is happening once every 1.748ms with NB_FRAMES = 16, NB_SAMPLES = 8
  // computed about 0.0413ms -> 41.3us per call overhead for OS required ~2.5% overhead
#endif

#if 1
  // demodulation handler based on microphone data coming in
  for( frames = 0; frames < NB_FRAMES; frames++ ) {
    FSKdemod(dm_buf + (frames * NB_SAMPLES), NB_SAMPLES, put_bit_mac);
  }
#endif
  data_ready = 0;

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
    log_ptr = 0;
    while( log_ptr < LOG_DEPTH ) {
      if( data_ready ) {
	// copy from the double-buffer into a demodulation buffer
	for( i = 0 ; i < buf_n; i++ ) { 
	  dm_buf[i] = (int16_t) (((int16_t) bufloc[i]) - 2048);
	}
	// call handler, which includes the demodulation routine
	adc_mic_handler(0);
      }
    }

#if !DEBUG_STREAMING
    for( i = 0; i < LOG_DEPTH; i++ ) {
      if( i % 16 == 0 )
	chprintf(stream, "\n\r" );
      chprintf(stream, "%02x", log_buf[i] /* isprint(log_buf[i]) ? log_buf[i] : '.'*/ );
      //      chprintf(stream, "%c", isprint(log_buf[i]) ? log_buf[i] : '.' );
    }

    // check hash
    MurmurHash3_x86_32(log_buf, LOG_DEPTH - 4 /* subtract hash itself from hash */, 0xdeadbeef, &hash);
    //    chprintf(stream, " hash: %02x%02x%02x%02x\n\r",
    //	     hash & 0xff, (hash >> 8) & 0xff, (hash >> 16) & 0xff, (hash >> 24) & 0xff);

    txhash = (log_buf[LOG_DEPTH-4] & 0xFF) | (log_buf[LOG_DEPTH-3] & 0xff) << 8 |
      (log_buf[LOG_DEPTH-2] & 0xFF) << 16 | (log_buf[LOG_DEPTH-1] & 0xff) << 24;

    chprintf(stream, " txhash: %08x\n\r", txhash);
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
  
  chprintf(stream, "\r\n\r\nOrchard shell.  Based on build %s\r\n", gitversion);
  print_mcu_info();

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

