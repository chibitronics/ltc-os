/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    crt0_v6m.s
 * @brief   Generic ARMv6-M (Cortex-M0/M1) startup file for ChibiOS.
 *
 * @addtogroup ARMCMx_GCC_STARTUP_V6M
 * @{
 */

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                               0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                                1
#endif

#define CONTROL_MODE_PRIVILEGED             0
#define CONTROL_MODE_UNPRIVILEGED           1
#define CONTROL_USE_MSP                     0
#define CONTROL_USE_PSP                     2

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Control special register initialization value.
 * @details The system is setup to run in privileged mode using the PSP
 *          stack (dual stack mode).
 */
#define CRT0_CONTROL_INIT                   (CONTROL_USE_PSP |              \
                                             CONTROL_MODE_PRIVILEGED)

/**
 * @brief   Constructors invocation switch.
 */
#define CRT0_CALL_CONSTRUCTORS              TRUE


/*===========================================================================*/
/* app_struct offsets                                                        */
/*===========================================================================*/
        .equ data_load_start,0x00   /* Start of data in ROM */
        .equ data_start,0x04        /* Start of data load address in RAM */
        .equ data_end,0x08          /* End of data load address in RAM */
        .equ bss_start,0x0c         /* Start of BSS in RAM */
        .equ bss_end,0x10           /* End of BSS in RAM */
        .equ entry,0x14             /* Address to jump to */
        .equ magic,0x18             /* 32-bit signature, defined below */
        .equ version,0x1c           /* BCD version number */
        .equ init_array_start,0x20  /* C++ constructor start */
        .equ init_array_end,0x24    /* C++ constructor end */
        .equ heap_start,0x28        /* Start of the heap offset */
        .equ heap_end,0x2c          /* End of the heap offset */


/*===========================================================================*/
/* Code section.                                                             */
/*===========================================================================*/

                .cpu    cortex-m0
                .fpu    softvfp

                .thumb
                .text

                .align  2
                .thumb_func
                .func Run_App
                .global Run_App
Run_App:

                /* Interrupts are globally masked initially.*/
                cpsid   i

                /* Save the struct in a register that won't get clobbered.*/
                mov     r7, r0

                /* PSP stack pointers initialization.*/
                ldr     r1, =__ram0_end__
                sub     r1, #4
                msr     PSP, r1

                /* CPU mode initialization as configured.*/
                movs    r1, #CRT0_CONTROL_INIT
                msr     CONTROL, r1
                isb

                /* Data initialization. Note, it assumes that the DATA size
                  is a multiple of 4 so the linker file must ensure this.*/
                ldr     r1, [r7, #data_load_start]
                ldr     r2, [r7, #data_start]
                ldr     r3, [r7, #data_end]
dloop:
                cmp     r2, r3
                bge     enddloop
                ldr     r4, [r1]
                str     r4, [r2]
                add     r1, r1, #4
                add     r2, r2, #4
                b       dloop
enddloop:

                /* BSS initialization. Note, it assumes that the DATA size
                  is a multiple of 4 so the linker file must ensure this.*/
                movs    r3, #0
                ldr     r1, [r7, #bss_start]
                ldr     r2, [r7, #bss_end]
bloop:
                cmp     r1, r2
                bge     endbloop
                str     r3, [r1]
                add     r1, r1, #4
                b       bloop
endbloop:

                /* Configure the heap */
                ldr     r1, [r7, #heap_start]
                ldr     r2, =os_heap_start
                str     r1, [r2]

                ldr     r1, [r7, #heap_end]
                ldr     r2, =os_heap_end
                str     r1, [r2]

                /* Interrupts must be enabled for SVC to work */
                /* Constructors may call SVC. */
                cpsie i

                /* Constructors invocation.*/
                ldr     r4, [r7, #init_array_start]
                ldr     r5, [r7, #init_array_end]
initloop:
                cmp     r4, r5
                bge     endinitloop
                ldr     r1, [r4]
                blx     r1
                add     r4, r4, #4
                b       initloop
endinitloop:

                /* Main program invocation, r7 contains the returned value.*/
                ldr     r7, [r7, #entry]
                bx      r7
                .endfunc
                .type Run_App, %function
                .size Run_App, .-Run_App

/** @} */
