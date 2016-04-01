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

#include "kl02x.h"

struct evt_table orchard_app_events;
event_source_t accel_event;
event_source_t accel_test_event;
event_source_t ta_update_event;
event_source_t adc_celcius_event;
event_source_t adc_mic_event;

char xp, yp, zp;

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

static void adc_mic_handler(eventid_t id) {
  (void) id;

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

  i2cStart(i2cDriver, &i2c_config);
  adcStart(&ADCD1, &adccfg1);
  analogStart();
  
  orchardShellInit();

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

