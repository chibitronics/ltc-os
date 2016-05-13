.macro export func
.thumb_func
.global \func
\func:
.endm

.macro double_wrapper1 basename
  .thumb_func
  .global qfp_d\basename
  qfp_d\basename:
   b qfp_f\basename
.endm

.macro double_wrapper2 basename
  .thumb_func
  .global __aeabi_d\basename
  .global qfp_d\basename
  __aeabi_d\basename:
  qfp_d\basename:
   mov r1, r3           @ Convert from 8-byte double to 4-byte float
   b qfp_f\basename

   .global __aeabi_f\basename
   __aeabi_f\basename:
    b qfp_f\basename
.endm

.macro double_wrapper3 basename
  .thumb_func
  .global qfp_\basename\()d
  qfp_\basename\()d:
   mov r1, r3           @ Convert from 8-byte double to 4-byte float
   b qfp_\basename
.endm

DOUBLE_WRAPPER3 cmpeq
DOUBLE_WRAPPER3 cmple
DOUBLE_WRAPPER3 cmplt
DOUBLE_WRAPPER3 cmpge
DOUBLE_WRAPPER3 cmpgt
DOUBLE_WRAPPER3 cmpun
DOUBLE_WRAPPER2 add
DOUBLE_WRAPPER2 sub
DOUBLE_WRAPPER2 mul
DOUBLE_WRAPPER2 div
DOUBLE_WRAPPER2 div_fast
DOUBLE_WRAPPER1 cos
DOUBLE_WRAPPER1 sin
DOUBLE_WRAPPER1 tan
DOUBLE_WRAPPER1 atan2
DOUBLE_WRAPPER1 exp
DOUBLE_WRAPPER1 ln
DOUBLE_WRAPPER1 sqrt
DOUBLE_WRAPPER1 sqrt_fast

.macro cmpfunc op, ty
.thumb_func
.global qfp_cmp\op
qfp_cmp\op:
  push {lr}
  bl qfp_fcmp
  cmp r0, #0
  b\ty 1f
  movs r0, #0
  pop {pc}
1:
  movs r0, #1
  pop {pc}
.endm

CMPFUNC eq,eq
CMPFUNC le,le
CMPFUNC lt,lt
CMPFUNC ge,ge
CMPFUNC gt,gt
CMPFUNC un,ne

.thumb_func
.global qfp_cfcmpeq
.global qfp_cfcmple
.global qfp_cfrcmple
qfp_cfrcmple:
  mov r2, r0
  mov r0, r1
  mov r1, r2      @ Swap and fall through
qfp_cfcmple:
qfp_cfcmpeq:
  push {lr}
  bl qfp_fcmp
  cmp r0, #0
  pop {pc}

.global qfp_cdcmpeq
.global qfp_cdcmple
.global qfp_cdrcmple
qfp_cdrcmple:
  mov r1, r0
  mov r0, r2
  b qfp_cfcmple

qfp_cdcmple:
qfp_cdcmpeq:
  mov r1, r2
  b qfp_cfcmple

.thumb_func
.global qfp_frsub
@ Equivalent to fsub, but reversed arguments
qfp_frsub:
 mov r2, r1
 mov r1, r0
 mov r0, r2
 b qfp_fsub

.thumb_func
.global qfp_drsub
@ Equivalent to dsub, but reversed arguments
qfp_drsub:
 mov r1, r0
 mov r0, r2
 b qfp_fsub

.thumb_func
.global qfp_double2int
qfp_double2int:
 b qfp_float2int

.thumb_func
.global qfp_double2uint
qfp_double2uint:
 b qfp_float2uint

.thumb_func
.global qfp_int2double
qfp_int2double:
 b qfp_int2float

.thumb_func
.global qfp_uint2double
qfp_uint2double:
 b qfp_uint2float

.thumb_func
.global qfp_double2str
qfp_double2str:
  mov r1, r2
  mov r2, r3
  b qfp_float2str

EXPORT qfp_str2double
  b qfp_str2double

EXPORT __aeabi_fcmplt
  b qfp_cmplt
