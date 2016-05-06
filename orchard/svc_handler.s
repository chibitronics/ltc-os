    /* SVC_Handler code adapted from ARM docs.
     * Determine what mode we're in, and load the appropriate stack to r7.
     * Then, load the svc number, which is stored on the stack.
     * Finally, use this as an index into SysCall_Table for dispatch.
     */
    .global SVC_Handler
    .func SVC_Handler
SVC_Handler:
    mov r7, lr
    mov r6, #4
    tst r7, r6
    beq svc_handler_msp
    bne svc_handler_psp
svc_handler_msp:
    mrs r7, MSP
    b svc_handler_dispatch
svc_handler_psp:
    mrs r7, PSP
    b svc_handler_dispatch
svc_handler_dispatch:
    mov     r6, #6*4
    ldr     r7, [r7, r6]
    mov     r6, #0
    sub     r6, #2
    ldrb    r7, [r7, r6]
    //svc_number = ((char *)svc_args[6])[-2];
    ldr     r6, =SysCall_Table
    lsl     r7, #2
    add     r6, r7, r6
    ldr     r6, [r6]
    bx      r6
    .endfunc
    .type SVC_Handler, %function
    .size SVC_Handler, .-SVC_Handler
