    /* SVC_Handler code adapted from ARM docs.
     * Determine what mode we're in, and load the appropriate stack to r0.
     * Then, load the svc number, which is not stored anywhere.  Instead,
     * we look at the return address (stored at -6 on the stack), and examine
     * the final byte of the opcode.
     * Finally, use this as an index into SysCall_Table for dispatch.
     * We then overwrite the return value, and return.
     */
    .global SVC_Handler
    .func SVC_Handler
SVC_Handler:
    mov r0, lr
    mov r1, #4
    tst r0, r1
    beq svc_handler_msp
    bne svc_handler_psp
svc_handler_msp:
    mrs r0, MSP
    b svc_handler_dispatch
svc_handler_psp:
    mrs r0, PSP
    b svc_handler_dispatch
    /* r0 now contains the stack:
    * [ r0, r1, r2, r3, r12, r14, the return address and xPSR ]
    */
svc_handler_dispatch:
    mov     r1, #6*4      // Get return address
    ldr     r2, [r0, r1]  // Save it in r1

    /* Get the svc number, which is stored in ram as the last byte */
    mov     r3, #0        // Index -2
    sub     r3, #2        // Decrement ot get to -2
    ldrb    r2, [r2, r3]  // Pull the last byte of the svc instruction into r2
    //svc_number = ((char *)svc_args[6])[-2];
    ldr     r3, =SysCall_Table  // Index from the start of SysCall_Table
    lsl     r2, #2              // Multiply svc instruction by 4
    ldr     r3, [r3, r2]        // Load the syscall address
    str     r3, [r0, r1]        // Store the return value back on the stack
    bx lr
    .endfunc
    .type SVC_Handler, %function
    .size SVC_Handler, .-SVC_Handler
