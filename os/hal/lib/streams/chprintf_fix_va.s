.section .text
.cpu cortex-m0plus
.fpu softvfp

ap      .req r0
faddr   .req r1
mask    .req r2
fwork   .req r3
.func va_arg_align_if_necessary
.global va_arg_align_if_necessary
va_arg_align_if_necessary:
    push {faddr, mask, fwork, lr}
    ldr faddr, [ap]
    mov fwork, faddr
    mov mask, #7
    and fwork, mask
    cmp fwork, #4
    beq return_value
    mov mask, #4
    add faddr, mask
    str faddr, [ap]
return_value:
    pop {r1, r2, r3, pc}

.endfunc
.type va_arg_align_if_necessary, %function
.size va_arg_align_if_necessary, .-va_arg_align_if_necessary