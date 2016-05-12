#include <stdint.h>
#include "Arduino.h"
extern "C" {
    extern uint32_t __aeabi_memcpy8;
    extern uint32_t __aeabi_memcpy4;
    extern uint32_t __aeabi_memcpy;
    extern uint32_t __aeabi_memmove8;
    extern uint32_t __aeabi_memmove4;
    extern uint32_t __aeabi_memmove;
    extern uint32_t __aeabi_memset8;
    extern uint32_t __aeabi_memset4;
    extern uint32_t __aeabi_memset;
    extern uint32_t __aeabi_memclr8;
    extern uint32_t __aeabi_memclr4;
    extern uint32_t __aeabi_memclr;
    extern uint32_t __eqdf2;
    extern uint32_t __gedf2;
    extern uint32_t __gtdf2;
    extern uint32_t __ledf2;
    extern uint32_t __ltdf2;
    extern uint32_t __nedf2;
    extern uint32_t __popcount_tab;
    extern uint32_t __popcountsi2;
    extern uint32_t __aeabi_cdcmpeq;
    extern uint32_t __aeabi_cdcmple;
    extern uint32_t __aeabi_cdrcmple;
    extern uint32_t __muldi3;
    extern uint32_t __aeabi_uidiv;
    extern uint32_t __aeabi_uidivmod;
    extern uint32_t __aeabi_uldivmod;
    extern uint32_t __aeabi_idiv;
    extern uint32_t __aeabi_idivmod;
    extern uint32_t __aeabi_lasr;
    extern uint32_t __aeabi_ldivmod;
    extern uint32_t __aeabi_llsl;
    extern uint32_t __aeabi_llsr;
    extern uint32_t __aeabi_lmul;
    extern uint32_t strcpy;
    extern uint32_t strlen;
    extern uint32_t free;
    extern uint32_t malloc;
    extern uint32_t realloc;
    extern uint32_t __dso_handle;
    extern uint32_t __aeabi_atexit;
    extern uint32_t __cxa_atexit;
    extern uint32_t memset;
    extern uint32_t ltoa;
    extern uint32_t utoa;
    extern uint32_t ultoa;
    extern uint32_t itoa;
    extern uint32_t ledShow;
    extern uint32_t tfp_printf;
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

extern "C" {
  extern const uint32_t * const SysCall_Table[];
};

__attribute__((section(".rodata")))
const uint32_t * const SysCall_Table[] = {
    (const uint32_t *)&__aeabi_memcpy8,
    (const uint32_t *)&__aeabi_memcpy4,
    (const uint32_t *)&__aeabi_memcpy,
    (const uint32_t *)&__aeabi_memmove8,
    (const uint32_t *)&__aeabi_memmove4,
    (const uint32_t *)&__aeabi_memmove,
    (const uint32_t *)&__aeabi_memset8,
    (const uint32_t *)&__aeabi_memset4,
    (const uint32_t *)&__aeabi_memset,
    (const uint32_t *)&__aeabi_memclr8,
    (const uint32_t *)&__aeabi_memclr4,
    (const uint32_t *)&__aeabi_memclr,
    (const uint32_t *)&__eqdf2,
    (const uint32_t *)&__gedf2,
    (const uint32_t *)&__gtdf2,
    (const uint32_t *)&__ledf2,
    (const uint32_t *)&__ltdf2,
    (const uint32_t *)&__nedf2,
    (const uint32_t *)&__popcount_tab,
    (const uint32_t *)&__popcountsi2,
    (const uint32_t *)&__aeabi_cdcmpeq,
    (const uint32_t *)&__aeabi_cdcmple,
    (const uint32_t *)&__aeabi_cdrcmple,
    (const uint32_t *)&__muldi3,
    (const uint32_t *)&__aeabi_uidiv,
    (const uint32_t *)&__aeabi_uidivmod,
    (const uint32_t *)&__aeabi_uldivmod,
    (const uint32_t *)&__aeabi_idiv,
    (const uint32_t *)&__aeabi_idivmod,
    (const uint32_t *)&__aeabi_lasr,
    (const uint32_t *)&__aeabi_ldivmod,
    (const uint32_t *)&__aeabi_llsl,
    (const uint32_t *)&__aeabi_llsr,
    (const uint32_t *)&__aeabi_lmul,
    (const uint32_t *)&strcpy,
    (const uint32_t *)&strlen,
    (const uint32_t *)&free,
    (const uint32_t *)&malloc,
    (const uint32_t *)&realloc,
    (const uint32_t *)&__dso_handle,
    (const uint32_t *)&__aeabi_atexit,
    (const uint32_t *)&__cxa_atexit,
    (const uint32_t *)&memset,
    (const uint32_t *)&ltoa,
    (const uint32_t *)&utoa,
    (const uint32_t *)&ultoa,
    (const uint32_t *)&itoa,
    (const uint32_t *)&ledShow,
    (const uint32_t *)&tfp_printf,
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
