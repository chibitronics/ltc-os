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

//// CONFIGURED FOR BW OLED VERSION OF PCB

#if HAL_USE_PAL || defined(__DOXYGEN__)
/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config =
{
  .ports = {
    {
      .port = IOPORT1,  // PORTA
      .pads = {
        /* PTA0*/ PAL_MODE_ALTERNATIVE_3 /* swc */,   /* PTA1*/ PAL_MODE_ALTERNATIVE_3,   /* PTA2*/ PAL_MODE_ALTERNATIVE_3, /*swd */
        /* PTA3*/ PAL_MODE_UNCONNECTED,     /* PTA4*/ PAL_MODE_UNCONNECTED,     /* PTA5*/ PAL_MODE_UNCONNECTED,
        /* PTA6*/ PAL_MODE_UNCONNECTED,   /* PTA7*/ PAL_MODE_UNCONNECTED,   /* PTA8*/ PAL_MODE_UNCONNECTED,
        /* PTA9*/ PAL_MODE_UNCONNECTED,    /*PTA10*/ PAL_MODE_INPUT_PULLUP,     /*PTA11*/ PAL_MODE_UNCONNECTED,
        /*PTA12*/ PAL_MODE_UNCONNECTED,    /*PTA13*/ PAL_MODE_UNCONNECTED,    /*PTA14*/ PAL_MODE_UNCONNECTED,
        /*PTA15*/ PAL_MODE_UNCONNECTED,    /*PTA16*/ PAL_MODE_UNCONNECTED,    /*PTA17*/ PAL_MODE_UNCONNECTED,
        /*PTA18*/ PAL_MODE_UNCONNECTED,    /*PTA19*/ PAL_MODE_UNCONNECTED,    /*PTA20*/ PAL_MODE_UNCONNECTED,
        /*PTA21*/ PAL_MODE_UNCONNECTED,     /*PTA22*/ PAL_MODE_UNCONNECTED,     /*PTA23*/ PAL_MODE_UNCONNECTED,
        /*PTA24*/ PAL_MODE_UNCONNECTED,     /*PTA25*/ PAL_MODE_UNCONNECTED,     /*PTA26*/ PAL_MODE_UNCONNECTED,
        /*PTA27*/ PAL_MODE_UNCONNECTED,     /*PTA28*/ PAL_MODE_UNCONNECTED,     /*PTA29*/ PAL_MODE_UNCONNECTED,
        /*PTA30*/ PAL_MODE_UNCONNECTED,     /*PTA31*/ PAL_MODE_UNCONNECTED,
      },
    },
    {
      .port = IOPORT2,  // PORTB
      .pads = {
	// set PTB1/2 to ALTERNATIVE_2 to swap UART TX/RX
        /* PTB0*/ PAL_MODE_UNCONNECTED,   /* PTB1*/ PAL_MODE_ALTERNATIVE_3, /*uart*/   /* PTB2*/ PAL_MODE_ALTERNATIVE_3, /*uart*/
        /* PTB3*/ PAL_MODE_ALTERNATIVE_2, /*i2c*/ /* PTB4*/ PAL_MODE_ALTERNATIVE_2, /*i2c*/   /* PTB5*/ PAL_MODE_INPUT_PULLUP, // NMI
        /* PTB6*/ PAL_MODE_OUTPUT_PUSHPULL, /*led R*/  /* PTB7*/ PAL_MODE_OUTPUT_PUSHPULL, /*led G*/  /* PTB8*/ PAL_MODE_UNCONNECTED,
        /* PTB9*/ PAL_MODE_UNCONNECTED,     /*PTB10*/ PAL_MODE_OUTPUT_PUSHPULL, /*led B*/  /*PTB11*/ PAL_MODE_UNCONNECTED,
        /*PTB12*/ PAL_MODE_UNCONNECTED,     /*PTB13*/ PAL_MODE_UNCONNECTED,  /*PTB14*/ PAL_MODE_UNCONNECTED,
        /*PTB15*/ PAL_MODE_UNCONNECTED,     /*PTB16*/ PAL_MODE_UNCONNECTED,      /*PTB17*/ PAL_MODE_UNCONNECTED,
        /*PTB18*/ PAL_MODE_UNCONNECTED,     /*PTB19*/ PAL_MODE_UNCONNECTED,     /*PTB20*/ PAL_MODE_UNCONNECTED,
        /*PTB21*/ PAL_MODE_UNCONNECTED,     /*PTB22*/ PAL_MODE_UNCONNECTED,     /*PTB23*/ PAL_MODE_UNCONNECTED,
        /*PTB24*/ PAL_MODE_UNCONNECTED,     /*PTB25*/ PAL_MODE_UNCONNECTED,     /*PTB26*/ PAL_MODE_UNCONNECTED,
        /*PTB27*/ PAL_MODE_UNCONNECTED,     /*PTB28*/ PAL_MODE_UNCONNECTED,     /*PTB29*/ PAL_MODE_UNCONNECTED,
        /*PTB30*/ PAL_MODE_UNCONNECTED,     /*PTB31*/ PAL_MODE_UNCONNECTED,
      },
    },
  },
};
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization must be performed just after stack setup
 *          and before any other initialization.
 */
void __early_init(void) {

  kl2x_clock_init();
}

/**
 * @brief   Board-specific initialization code.
 * @todo    Add your board-specific code, if any.
 */
void boardInit(void) {
}
