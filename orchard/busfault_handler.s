     /* Determine what mode we're in, and load the appropriate stack to sp. */
    .global BusFault_Handler
    .func BusFault_Handler
BusFault_Handler:

#ifdef ENABLE_BKPT
    mov r7, lr
    mov r6, #4
    tst r7, r6
    beq busfault_handler_msp
    bne busfault_handler_psp

busfault_handler_msp:
    mrs r0, MSP
    mov r1, #1
    mov sp, r0          // Restore the stack pointer

    ldr r2, =BusFault_Handler_C
    bx r2

busfault_handler_psp:
    mrs r0, PSP
    mov r1, #0          // Set "is_isr" to 0
    mov sp, r0          // Restore the stack pointer so "bt" works

    ldr r2, =BusFault_Handler_C
    bx r2
#endif /* ENABLE_BKPT */

    ldr r2, =errorCondition
    bx r2
    .endfunc
    .type BusFault_Handler, %function
    .size BusFault_Handler, .-BusFault_Handler
