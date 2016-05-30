    /* HardFault_Handler code adapted from ARM docs.
     * Determine what mode we're in, and load the appropriate stack to sp.
     */
    .global HardFault_Handler
    .func HardFault_Handler
HardFault_Handler:
    mov r3, lr
    mov r2, #4
    tst r3, r2
    beq hardfault_handler_msp
    bne hardfault_handler_psp

hardfault_handler_msp:
    mrs r3, MSP
    mov r0, r3
    mov r1, #1

    mov sp, r0          // Restore the stack pointer

    ldr r2, =HardFault_Handler_C
    bx r2

hardfault_handler_psp:
    mrs r3, PSP
    mov r0, r3          // Store the stack frame in register 0
    mov r1, #0          // Set "is_isr" to 0
    mov sp, r0          // Restore the stack pointer so "bt" works

    ldr r2, =HardFault_Handler_C
    bx r2
    .endfunc
    .type HardFault_Handler, %function
    .size HardFault_Handler, .-HardFault_Handler
