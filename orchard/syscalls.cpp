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
uint32_t *SysCall_Table[] = {
    (uint32_t *)&__aeabi_memcpy8,
    (uint32_t *)&__aeabi_memcpy4,
    (uint32_t *)&__aeabi_memcpy,
    (uint32_t *)&__aeabi_memmove8,
    (uint32_t *)&__aeabi_memmove4,
    (uint32_t *)&__aeabi_memmove,
    (uint32_t *)&__aeabi_memset8,
    (uint32_t *)&__aeabi_memset4,
    (uint32_t *)&__aeabi_memset,
    (uint32_t *)&__aeabi_memclr8,
    (uint32_t *)&__aeabi_memclr4,
    (uint32_t *)&__aeabi_memclr,
    (uint32_t *)&__eqdf2,
    (uint32_t *)&__gedf2,
    (uint32_t *)&__gtdf2,
    (uint32_t *)&__ledf2,
    (uint32_t *)&__ltdf2,
    (uint32_t *)&__nedf2,
    (uint32_t *)&__popcount_tab,
    (uint32_t *)&__popcountsi2,
    (uint32_t *)&__aeabi_cdcmpeq,
    (uint32_t *)&__aeabi_cdcmple,
    (uint32_t *)&__aeabi_cdrcmple,
    (uint32_t *)&__muldi3,
    (uint32_t *)&__aeabi_uidiv,
    (uint32_t *)&__aeabi_uidivmod,
    (uint32_t *)&__aeabi_uldivmod,
    (uint32_t *)&__aeabi_idiv,
    (uint32_t *)&__aeabi_idivmod,
    (uint32_t *)&__aeabi_lasr,
    (uint32_t *)&__aeabi_ldivmod,
    (uint32_t *)&__aeabi_llsl,
    (uint32_t *)&__aeabi_llsr,
    (uint32_t *)&__aeabi_lmul,
    (uint32_t *)static_cast<void (*)(int pin, enum pin_mode mode)>(&pinMode),
    (uint32_t *)static_cast<void (*)(int pin, int value)>(&digitalWrite),
    (uint32_t *)static_cast<int (*)(int pin)>(&digitalRead),
    (uint32_t *)static_cast<void (*)(int pin, int value)>(&analogWrite),
    (uint32_t *)static_cast<void (*)(enum analog_reference_type type)>(&analogReference),
    (uint32_t *)static_cast<int (*)(int pin)>(&analogRead),
    (uint32_t *)static_cast<void (*)(int irq, void (*func)(void), enum irq_mode mode)>(&attachInterrupt),
    (uint32_t *)static_cast<void (*)(int irq)>(&detachInterrupt),
    (uint32_t *)static_cast<void (*)(int pin, unsigned int frequency, unsigned long duration)>(&tone),
    (uint32_t *)static_cast<void (*)(int pin)>(&noTone),
    (uint32_t *)static_cast<void (*)(int dataPin, int clockPin, int bitOrder, uint8_t val)>(&shiftOut),
    (uint32_t *)static_cast<uint8_t (*)(int dataPin, int clockPin, int bitOrder)>(&shiftIn),
    (uint32_t *)static_cast<unsigned long (*)(int pin, uint8_t state, unsigned long timeout)>(&pulseIn),
    (uint32_t *)static_cast<unsigned long (*)(int pin, uint8_t state, unsigned long timeout)>(&pulseInLong),
    (uint32_t *)static_cast<unsigned long (*)(void)>(&millis),
    (uint32_t *)static_cast<unsigned long (*)(void)>(&micros),
    (uint32_t *)static_cast<void (*)(unsigned long msecs)>(&delay),
    (uint32_t *)static_cast<void (*)(unsigned int usecs)>(&delayMicroseconds),
    (uint32_t *)static_cast<long (*)(long value, long fromLow, long fromHigh, long toLow, long toHigh)>(&map),
    (uint32_t *)static_cast<long (*)(long min, long max)>(&random),
    (uint32_t *)static_cast<long (*)(unsigned long seed)>(&randomSeed),
};
