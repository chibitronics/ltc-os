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

