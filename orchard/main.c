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

#include "oled.h"
#include "analog.h"

#include "gfx.h"

#include "dv-serialmode.h"

#include "kl02x.h"

struct evt_table orchard_app_events;
event_source_t refresh_event;
event_source_t serial_event;
uint8_t serial_init = 0;

static virtual_timer_t led_vt;
static virtual_timer_t refresh_vt;

static void mode_cb(EXTDriver *extp, expchannel_t channel);
static void option_cb(EXTDriver *extp, expchannel_t channel);

static const SPIConfig spi_config = {
  NULL,
  /* HW dependent part.*/
  GPIOA,
  5,
};

static const ADCConfig adccfg1 = {
  /* Perform initial calibration */
  true
};

static const EXTConfig ext_config = {
  {
    {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, mode_cb, PORTB, 6},
    {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, option_cb, PORTB, 7},
  }
};

typedef enum {
  MODE_SERIAL,
  MODE_OSCOPE,
  MODE_VOLTS,
} dv_mode;
static char current_mode = MODE_SERIAL;

#define REFRESH_RATE  50   // in ms

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

static void led_cb(void *arg) {
  (void) arg;

  palTogglePad(GPIOA, 6);
  chSysLockFromISR();
  chVTSetI(&led_vt, MS2ST(500), led_cb, NULL);
  chSysUnlockFromISR();
}

static void refresh_cb(void *arg) {
  (void) arg;

  chSysLockFromISR();
  chVTSetI(&refresh_vt, MS2ST(REFRESH_RATE), refresh_cb, NULL);
  chEvtBroadcastI(&refresh_event);
  chSysUnlockFromISR();
}

static void mode_cb(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;
}


static void option_cb(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;
}

static void adc_celcius_handler(eventid_t id) {
  (void) id;
  chprintf(stream, "celcius: %d\n\r", analogReadTemperature());
}

static void refresh_handler(eventid_t id) {
  (void) id;

  switch(current_mode) {
  case MODE_SERIAL:
    updateSerialScreen();
    break;
  default:
    break;
  }
}

static void serial_handler(eventid_t id) {
  (void) id;
  dvDoSerial();
}

static thread_t *evHandler_tp = NULL;
static THD_WORKING_AREA(waEvHandlerThread, 0x500);

static THD_FUNCTION(evHandlerThread, arg) {
  (void)arg;
  chRegSetThreadName("Event dispatcher");

  adcStart(&ADCD1, &adccfg1);
  analogStart();

  spiStart(&SPID1, &spi_config);
  oledStart(&SPID1);

  orchardShellInit();

  chprintf(stream, "\r\n\r\nOrchard shell.  Based on build %s\r\n", gitversion);
  print_mcu_info();
  chprintf(stream, "free memory at boot: %d bytes\r\n", chCoreGetStatusX());

  evtTableInit(orchard_app_events, 9);

  chEvtObjectInit(&refresh_event);
  evtTableHook(orchard_app_events, refresh_event, refresh_handler);

  chEvtObjectInit(&serial_event);
  evtTableHook(orchard_app_events, serial_event, serial_handler);

  extStart(&EXTD1, &ext_config); // enables interrupts on gpios

  // now handled properly elsewhere
  // palWritePad(GPIOA, 5, PAL_HIGH);  // oled_cs
  // palWritePad(GPIOB, 13,PAL_HIGH);  // oled_dc

  //  palWritePad(GPIOB, 11,PAL_HIGH);  // oled_res
  orchardGfxInit();
  oledOrchardBanner();
  
  chprintf(stream, "free memory after gfx init: %d bytes\r\n", chCoreGetStatusX());

  // start refreshing the screen
  chVTObjectInit(&refresh_vt);
  chVTSet(&refresh_vt, MS2ST(REFRESH_RATE), refresh_cb, NULL);

  serial_init = 1;

  dvInit();
  while(true) {
    chEvtDispatch(evtHandlers(orchard_app_events), chEvtWaitOne(ALL_EVENTS));
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

  // set this low here in case something crashes later on, at least we have the LED on to indicate power-on
  palWritePad(GPIOA, 6, PAL_LOW); // mcu_led

  chVTObjectInit(&led_vt);
  chVTSet(&led_vt, MS2ST(500), led_cb, NULL);

  evHandler_tp = chThdCreateStatic(waEvHandlerThread, sizeof(waEvHandlerThread), NORMALPRIO + 10, evHandlerThread, NULL);

  while (TRUE) {
    // this is now an idle loop
  }

}

