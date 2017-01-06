.section .rodata
.global SysCall_Table
SysCall_Table:

.short do_nothing
.short getSyscallABI
.short getSyscallAddr
.short memcpy
.short memmove
.short memset
.short memclr
.short __aeabi_uread4
.short __aeabi_uwrite4
.short __aeabi_uread8
.short __aeabi_idiv
.short __aeabi_uidiv
.short __aeabi_idivmod
.short __aeabi_uidivmod
.short __aeabi_lmul
.short __aeabi_ldivmod
.short __aeabi_uldivmod
.short __aeabi_llsl
.short __aeabi_llsr
.short __aeabi_lasr
.short __aeabi_lcmp
.short __aeabi_ulcmp
.short qfp_int2float
.short qfp_int2double
.short qfp_uint2float
.short qfp_uint2double
.short qfp_float2int
.short qfp_double2int
.short qfp_float2uint
.short qfp_double2uint
.short qfp_cfcmpeq
.short qfp_cdcmpeq
.short qfp_cfcmple
.short qfp_cdcmple
.short qfp_cfrcmple
.short qfp_cdrcmple
.short qfp_cmpeq
.short qfp_cmpeqd
.short qfp_cmplt
.short qfp_cmpltd
.short qfp_cmple
.short qfp_cmpled
.short qfp_cmpge
.short qfp_cmpged
.short qfp_cmpgt
.short qfp_cmpgtd
.short qfp_cmpun
.short qfp_cmpund
.short qfp_fadd
.short qfp_dadd
.short qfp_fdiv_fast
.short qfp_ddiv_fast
.short qfp_fmul
.short qfp_dmul
.short qfp_frsub
.short qfp_drsub
.short qfp_fsub
.short qfp_dsub
.short qfp_float2str
.short qfp_str2float
.short qfp_fcos
.short qfp_fsin
.short qfp_ftan
.short qfp_fatan2
.short qfp_fexp
.short qfp_fln
.short qfp_fsqrt_fast
.short strncpy
.short strcpy
.short strcmp
.short strncmp
.short strchr
.short strlen
.short chsnprintf
.short ltoa
.short utoa
.short ultoa
.short itoa
.short simple_strtol
.short simple_strtoul
.short printf
.short putchar
.short getchar
.short cangetchar
.short free
.short malloc
.short realloc
.short ledShow
.short pinMode
.short digitalWrite
.short digitalRead
.short analogWrite
.short analogReference
.short analogRead
.short attachInterrupt
.short attachFastInterrupt
.short detachInterrupt
.short detachFastInterrupt
.short tone
.short noTone
.short shiftOut
.short shiftIn
.short pulseIn
.short pulseInLong
.short millis
.short micros
.short delay
.short delayMicroseconds
.short map
.short random
.short randomSeed
.short i2cTransceive
.short i2cSetupSlave
.short 0
.short 0
.short 0
.short 0
.short 0
.short createThread
.short exitThread
.short suspendThread
.short suspendThreadTimeout
.short resumeThread
.short yieldThread
.short threadSleep
.short threadSleepUntil
.short waitThread
.short setTimer
.short resetTimer
.short runCallbacks
.short lockSystem
.short unlockSystem
.short lockSystemFromISR
.short unlockSystemFromISR
.short mutexInit
.short mutexLock
.short mutexTryLock
.short mutexUnlock
.short flashErase
.short flashWrite
.short __gnu_thumb1_case_sqi
.short __gnu_thumb1_case_uqi
.short __gnu_thumb1_case_uhi
.short doSudo
.short setSerialSpeed
.short setThreadName
.short hookSysTick
.short resumeThreadI
.short suspendThreadS
.short suspendThreadTimeoutS
.short avoidIsrLock
.size SysCall_Table, .-SysCall_Table
