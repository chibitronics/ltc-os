/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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
#include "shell.h"
#include "chprintf.h"

#include "orchard-shell.h"

#include "analog.h"
#include "demod.h"
#include "mac.h"

void cmd_celcius(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: celcius\r\n");
    return;
  }
  
  analogUpdateTemperature();
  
}

orchard_command("celcius", cmd_celcius);

void cmd_adc(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: celcius\r\n");
    return;
  }
  
  analogUpdateMic();
  
}

orchard_command("adc", cmd_adc);

#if 0
void cmd_dump(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint32_t i;
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: dump\r\n");
    return;
  }

  dm_buf_ptr = 0;
  while( dm_buf_ptr < DMBUF_DEPTH ) {
    chThdSleepMilliseconds(100);
  }
  
  for( i = 0; i < DMBUF_DEPTH; i++ ) {
    if( i % 16 == 0 )
      chprintf(chp, "\n\r" );
    chprintf(chp, "%4d ", dm_buf[i]);
  }
  
}

orchard_command("dump", cmd_dump);
#endif

void cmd_demod(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint32_t i;
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: demod\r\n");
    return;
  }

  pktPtr = 0;
  while( pktPtr < PKT_LEN ) {
    //    chThdSleepMilliseconds(200);
  }
  
  for( i = 0; i < PKT_LEN; i++ ) {
    if( i % 16 == 0 )
      chprintf(chp, "\n\r" );
    chprintf(chp, "%2x ", pktBuf[i] /* isprint(pktBuf[i]) ? pktBuf[i] : '.'*/ );
  }
  
}

orchard_command("demod", cmd_demod);


void cmd_d(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint32_t i;
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: d\r\n");
    return;
  }

  pktPtr = 0;
  while( pktPtr < PKT_LEN ) {
    //    chThdSleepMilliseconds(200);
  }
  
  for( i = 0; i < PKT_LEN; i++ ) {
    if( i % 16 == 0 )
      chprintf(chp, "\n\r" );
    chprintf(chp, "%c", isprint(pktBuf[i]) ? pktBuf[i] : '.' );
  }
  
}

orchard_command("d", cmd_d);


void cmd_kill(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: demod\r\n");
    return;
  }

  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
    
  nvicDisableVector(HANDLER_SYSTICK);
}

orchard_command("kill", cmd_kill);

