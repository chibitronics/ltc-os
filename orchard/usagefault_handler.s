     /* Determine what mode we're in, and load the appropriate stack to sp. */
    .global UsageFault_Handler
    .func UsageFault_Handler
UsageFault_Handler:
    mov r7, lr
    mov r6, #4
    tst r7, r6
    beq usagefault_handler_msp
    bne usagefault_handler_psp

usagefault_handler_msp:
    mrs r0, MSP
    mov r1, #1

    mov sp, r0          // Restore the stack pointer

    ldr r2, =UsageFault_Handler_C
    bx r2

usagefault_handler_psp:
    mrs r0, PSP
    mov r1, #0          // Set "is_isr" to 0
    mov sp, r0          // Restore the stack pointer so "bt" works

    ldr r2, =UsageFault_Handler_C
    bx r2
    .endfunc
    .type UsageFault_Handler, %function
    .size UsageFault_Handler, .-UsageFault_Handler
