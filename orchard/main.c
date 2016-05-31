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
#include "dv-volts.h"
#include "dv-oscope.h"

#include "kl02x.h"

struct evt_table orchard_app_events;
event_source_t refresh_event;
event_source_t serial_event;
event_source_t mode_event;
event_source_t option_event;
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
  chSysLockFromISR();
  chEvtBroadcastI(&mode_event);
  chSysUnlockFromISR();
}


static void option_cb(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;
  chSysLockFromISR();
  chEvtBroadcastI(&option_event);
  chSysUnlockFromISR();
}

static void refresh_handler(eventid_t id) {
  (void) id;

  switch(current_mode) {
  case MODE_SERIAL:
    updateSerialScreen();
    break;
  case MODE_VOLTS:
    updateVoltsScreen();
    break;
  case MODE_OSCOPE:
    updateOscopeScreen();
    break;
  default:
    break;
  }
}

static void option_handler(eventid_t id) {
  (void) id;
  
  switch(current_mode) {
  case MODE_SERIAL:
    serial_init = serial_init ? 0 : 1;  // "scroll lock"
    if( !serial_init  )
      oledPauseBanner("Serial Paused");
    else
      oledPauseBanner("Serial Resumed");
    break;
  case MODE_VOLTS:
    // no modal behavior
    break;
  case MODE_OSCOPE:
    speed_mode = speed_mode ? 0 : 1; // toggle speed mode
    if( speed_mode ) 
      oledPauseBanner("High speed");
    else
      oledPauseBanner("Low speed");
    break;
  default:
    break;
  }
}

static void serial_handler(eventid_t id) {
  (void) id;
  dvDoSerial();
}

static void mode_handler(eventid_t id) {
  (void) id;
  switch(current_mode) {
  case MODE_SERIAL:
    oledPauseBanner("Wave Mode");
    serial_init = 0;
    current_mode = MODE_OSCOPE;
    break;
  case MODE_OSCOPE:
    oledPauseBanner("Volts Mode");
    current_mode = MODE_VOLTS;
    break;
  case MODE_VOLTS:
    oledPauseBanner("Serial Mode");
    serial_init = 1;
    current_mode = MODE_SERIAL;
    break;
  default:
    osalDbgAssert(false, "invalid operating mode");
  }
}

static thread_t *evHandler_tp = NULL;
static THD_WORKING_AREA(waEvHandlerThread, 0x480);

static THD_FUNCTION(evHandlerThread, arg) {
  (void)arg;
  chRegSetThreadName("Event dispatcher");

  adcStart(&ADCD1, &adccfg1);
  analogStart();

  spiStart(&SPID1, &spi_config);
  oledStart(&SPID1);

  orchardShellInit();

  chprintf(stream, "\r\nChibitronics Dataviewer build %s\r\n", gitversion);
  chprintf(stream, "boot freemem: %d\r\n", chCoreGetStatusX());

  evtTableInit(orchard_app_events, 6);

  chEvtObjectInit(&mode_event);
  evtTableHook(orchard_app_events, mode_event, mode_handler);

  chEvtObjectInit(&option_event);
  evtTableHook(orchard_app_events, option_event, option_handler);

  chEvtObjectInit(&refresh_event);
  evtTableHook(orchard_app_events, refresh_event, refresh_handler);

  chEvtObjectInit(&serial_event);
  current_mode = MODE_SERIAL;
  evtTableHook(orchard_app_events, serial_event, serial_handler);

  extStart(&EXTD1, &ext_config); // enables interrupts on gpios

  // now handled properly elsewhere
  // palWritePad(GPIOA, 5, PAL_HIGH);  // oled_cs
  // palWritePad(GPIOB, 13,PAL_HIGH);  // oled_dc

  //  palWritePad(GPIOB, 11,PAL_HIGH);  // oled_res
  orchardGfxInit();
  // oledOrchardBanner();
  
  chprintf(stream, "after gfx mem: %d bytes\r\n", chCoreGetStatusX());

  // start LED flashing
  chVTObjectInit(&led_vt);
  chVTSet(&led_vt, MS2ST(500), led_cb, NULL);

  // start refreshing the screen
  chVTObjectInit(&refresh_vt);
  chVTSet(&refresh_vt, MS2ST(REFRESH_RATE), refresh_cb, NULL);

  serial_init = 1;
  oledPauseBanner("Serial Mode");

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

  evHandler_tp = chThdCreateStatic(waEvHandlerThread, sizeof(waEvHandlerThread), NORMALPRIO + 10, evHandlerThread, NULL);

  while (TRUE) {
    // this is now an idle loop
  }

}

