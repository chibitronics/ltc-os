#include <stdint.h>
#include "Arduino.h"

extern "C" {
  extern const uint32_t * const SysCall_Table[];
  extern uint32_t do_nothing;
  extern uint32_t __aeabi_memcpy;
  extern uint32_t __aeabi_memmove;
  extern uint32_t __aeabi_memset;
  extern uint32_t __aeabi_memclr;
  extern uint32_t __aeabi_uread4;
  extern uint32_t __aeabi_uwrite4;
  extern uint32_t __aeabi_uread8;
  extern uint32_t __aeabi_idiv;
  extern uint32_t __aeabi_uidiv;
  extern uint32_t __aeabi_idivmod;
  extern uint32_t __aeabi_uidivmod;
  extern uint32_t __aeabi_lmul;
  extern uint32_t __aeabi_ldivmod;
  extern uint32_t __aeabi_uldivmod;
  extern uint32_t __aeabi_llsl;
  extern uint32_t __aeabi_llsr;
  extern uint32_t __aeabi_lasr;
  extern uint32_t __aeabi_lcmp;
  extern uint32_t __aeabi_ulcmp;
  extern uint32_t qfp_int2float;
  extern uint32_t qfp_int2double;
  extern uint32_t qfp_uint2float;
  extern uint32_t qfp_uint2double;
  extern uint32_t qfp_float2int;
  extern uint32_t qfp_double2int;
  extern uint32_t qfp_float2uint;
  extern uint32_t qfp_double2uint;
  extern uint32_t qfp_cfcmpeq;
  extern uint32_t qfp_cdcmpeq;
  extern uint32_t qfp_cfcmple;
  extern uint32_t qfp_cdcmple;
  extern uint32_t qfp_cfrcmple;
  extern uint32_t qfp_cdrcmple;
  extern uint32_t qfp_cmpeq;
  extern uint32_t qfp_cmpeqd;
  extern uint32_t qfp_cmplt;
  extern uint32_t qfp_cmpltd;
  extern uint32_t qfp_cmple;
  extern uint32_t qfp_cmpled;
  extern uint32_t qfp_cmpge;
  extern uint32_t qfp_cmpged;
  extern uint32_t qfp_cmpgt;
  extern uint32_t qfp_cmpgtd;
  extern uint32_t qfp_cmpun;
  extern uint32_t qfp_cmpund;
  extern uint32_t qfp_fadd;
  extern uint32_t qfp_dadd;
  extern uint32_t qfp_fdiv_fast;
  extern uint32_t qfp_ddiv_fast;
  extern uint32_t qfp_fmul;
  extern uint32_t qfp_dmul;
  extern uint32_t qfp_frsub;
  extern uint32_t qfp_drsub;
  extern uint32_t qfp_fsub;
  extern uint32_t qfp_dsub;
  extern uint32_t qfp_fcos;
  extern uint32_t qfp_fsin;
  extern uint32_t qfp_ftan;
  extern uint32_t qfp_fatan2;
  extern uint32_t qfp_fexp;
  extern uint32_t qfp_fln;
  extern uint32_t qfp_fsqrt_fast;
  extern uint32_t free;
  extern uint32_t malloc;
  extern uint32_t realloc;
  extern uint32_t __dso_handle;
  extern uint32_t ltoa;
  extern uint32_t utoa;
  extern uint32_t ultoa;
  extern uint32_t itoa;
  extern uint32_t ledShow;
  extern uint32_t printf;
};

extern void pinMode(int pin, enum pin_mode mode);
extern void digitalWrite(int pin, int value);
extern int digitalRead(int pin);
extern void analogWrite(int pin, int value);
extern void analogReference(enum analog_reference_type type);
extern int analogRead(int pin);
extern void attachInterrupt(int irq, void (*func)(void), enum irq_mode mode);
extern void detachInterrupt(int irq);
extern void tone(int pin, unsigned int frequency, unsigned long duration);
extern void noTone(int pin);
extern void shiftOut(int dataPin, int clockPin, int bitOrder, uint8_t val);
extern uint8_t shiftIn(int dataPin, int clockPin, int bitOrder);
extern unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout);
extern unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout);
extern unsigned long millis(void);
extern unsigned long micros(void);
extern void delay(unsigned long msecs);
extern void delayMicroseconds(unsigned int usecs);
extern long map(long value, long fromLow, long fromHigh, long toLow, long toHigh);
extern long random(long min, long max);
extern long randomSeed(unsigned long seed);

__attribute__((section(".rodata")))
const uint32_t * const SysCall_Table[] = {
    (const uint32_t *)&do_nothing,
    (const uint32_t *)&__aeabi_memcpy,
    (const uint32_t *)&__aeabi_memmove,
    (const uint32_t *)&__aeabi_memset,
    (const uint32_t *)&__aeabi_memclr,
    (const uint32_t *)&__aeabi_uread4,
    (const uint32_t *)&__aeabi_uwrite4,
    (const uint32_t *)&__aeabi_uread8,
    (const uint32_t *)&__aeabi_idiv,
    (const uint32_t *)&__aeabi_uidiv,
    (const uint32_t *)&__aeabi_idivmod,
    (const uint32_t *)&__aeabi_uidivmod,
    (const uint32_t *)&__aeabi_lmul,
    (const uint32_t *)&__aeabi_ldivmod,
    (const uint32_t *)&__aeabi_uldivmod,
    (const uint32_t *)&__aeabi_llsl,
    (const uint32_t *)&__aeabi_llsr,
    (const uint32_t *)&__aeabi_lasr,
    (const uint32_t *)&__aeabi_lcmp,
    (const uint32_t *)&__aeabi_ulcmp,
    (const uint32_t *)&qfp_int2float,
    (const uint32_t *)&qfp_int2double,
    (const uint32_t *)&qfp_uint2float,
    (const uint32_t *)&qfp_uint2double,
    (const uint32_t *)&qfp_float2int,
    (const uint32_t *)&qfp_double2int,
    (const uint32_t *)&qfp_float2uint,
    (const uint32_t *)&qfp_double2uint,
    (const uint32_t *)&qfp_cfcmpeq,
    (const uint32_t *)&qfp_cdcmpeq,
    (const uint32_t *)&qfp_cfcmple,
    (const uint32_t *)&qfp_cdcmple,
    (const uint32_t *)&qfp_cfrcmple,
    (const uint32_t *)&qfp_cdrcmple,
    (const uint32_t *)&qfp_cmpeq,
    (const uint32_t *)&qfp_cmpeqd,
    (const uint32_t *)&qfp_cmplt,
    (const uint32_t *)&qfp_cmpltd,
    (const uint32_t *)&qfp_cmple,
    (const uint32_t *)&qfp_cmpled,
    (const uint32_t *)&qfp_cmpge,
    (const uint32_t *)&qfp_cmpged,
    (const uint32_t *)&qfp_cmpgt,
    (const uint32_t *)&qfp_cmpgtd,
    (const uint32_t *)&qfp_cmpun,
    (const uint32_t *)&qfp_cmpund,
    (const uint32_t *)&qfp_fadd,
    (const uint32_t *)&qfp_dadd,
    (const uint32_t *)&qfp_fdiv_fast,
    (const uint32_t *)&qfp_ddiv_fast,
    (const uint32_t *)&qfp_fmul,
    (const uint32_t *)&qfp_dmul,
    (const uint32_t *)&qfp_frsub,
    (const uint32_t *)&qfp_drsub,
    (const uint32_t *)&qfp_fsub,
    (const uint32_t *)&qfp_dsub,
    (const uint32_t *)&qfp_fcos,
    (const uint32_t *)&qfp_fsin,
    (const uint32_t *)&qfp_ftan,
    (const uint32_t *)&qfp_fatan2,
    (const uint32_t *)&qfp_fexp,
    (const uint32_t *)&qfp_fln,
    (const uint32_t *)&qfp_fsqrt_fast,
    (const uint32_t *)&free,
    (const uint32_t *)&malloc,
    (const uint32_t *)&realloc,
    (const uint32_t *)&__dso_handle,
    (const uint32_t *)&ltoa,
    (const uint32_t *)&utoa,
    (const uint32_t *)&ultoa,
    (const uint32_t *)&itoa,
    (const uint32_t *)&ledShow,
    (const uint32_t *)&printf,
    (const uint32_t *)static_cast<void (*)(int pin, enum pin_mode mode)>(&pinMode),
    (const uint32_t *)static_cast<void (*)(int pin, int value)>(&digitalWrite),
    (const uint32_t *)static_cast<int (*)(int pin)>(&digitalRead),
    (const uint32_t *)static_cast<void (*)(int pin, int value)>(&analogWrite),
    (const uint32_t *)static_cast<void (*)(enum analog_reference_type type)>(&analogReference),
    (const uint32_t *)static_cast<int (*)(int pin)>(&analogRead),
    (const uint32_t *)static_cast<void (*)(int irq, void (*func)(void), enum irq_mode mode)>(&attachInterrupt),
    (const uint32_t *)static_cast<void (*)(int irq)>(&detachInterrupt),
    (const uint32_t *)static_cast<void (*)(int pin, unsigned int frequency, unsigned long duration)>(&tone),
    (const uint32_t *)static_cast<void (*)(int pin)>(&noTone),
    (const uint32_t *)static_cast<void (*)(int dataPin, int clockPin, int bitOrder, uint8_t val)>(&shiftOut),
    (const uint32_t *)static_cast<uint8_t (*)(int dataPin, int clockPin, int bitOrder)>(&shiftIn),
    (const uint32_t *)static_cast<unsigned long (*)(int pin, uint8_t state, unsigned long timeout)>(&pulseIn),
    (const uint32_t *)static_cast<unsigned long (*)(int pin, uint8_t state, unsigned long timeout)>(&pulseInLong),
    (const uint32_t *)static_cast<unsigned long (*)(void)>(&millis),
    (const uint32_t *)static_cast<unsigned long (*)(void)>(&micros),
    (const uint32_t *)static_cast<void (*)(unsigned long msecs)>(&delay),
    (const uint32_t *)static_cast<void (*)(unsigned int usecs)>(&delayMicroseconds),
    (const uint32_t *)static_cast<long (*)(long value, long fromLow, long fromHigh, long toLow, long toHigh)>(&map),
    (const uint32_t *)static_cast<long (*)(long min, long max)>(&random),
    (const uint32_t *)static_cast<long (*)(unsigned long seed)>(&randomSeed),
};
