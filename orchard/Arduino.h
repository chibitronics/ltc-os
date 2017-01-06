#ifndef __ARDUINO_KOSAGI_H__
#define __ARDUINO_KOSAGI_H__

#include "hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "kl02.h"

enum analog_reference_type {
  DEFAULT = 0,
  INTERNAL = 1,
  INTERNAL1V1 = 2,
  INTERNAL2V56 = 3,
  EXTERNAL = 4,
};

enum irq_mode {
  LOW = 0,
  CHANGE = 1,
  RISING = 2,
  FALLING = 3,
  HIGH = 4,
};

enum pin_mode {
  INPUT = 0,
  OUTPUT = 1,
  INPUT_PULLUP = 2,
  INPUT_PULLDOWN = 3,
};

#define LSBFIRST 0
#define MSBFIRST 1

#define digitalPinToPort(x) (((x & 0x3f) < 32) ? FGPIOA_BASE : FGPIOB_BASE)
#define digitalPinToBitMask(x) (1 << (x & 0x1f))

/* Program lifetime */
#if !defined(ARDUINO_OS) /* These are not syscalls */
#ifdef __cplusplus
extern "C" {
#endif
void init(void);
void loop(void);
void setup(void);
#ifdef __cplusplus
};
#endif
#endif /* !ARDUINO_OS */

/* Generic IO */
void pinMode(int pin, enum pin_mode mode);

/* Digital IO */
void digitalWrite(int pin, int value);
int digitalRead(int pin);

/* Analog IO */
void analogWrite(int pin, int value);
void analogReference(enum analog_reference_type type);
int analogRead(int pin);

/* Interrupt handling */
void attachInterrupt(int irq, void (*func)(void), enum irq_mode mode);
void detachInterrupt(int irq);

/* Tone production */
void tone(int pin, unsigned int frequency, unsigned long duration
#if !defined(ARDUINO_OS) && defined(__cplusplus)
    = 0
#endif /* !ARDUINO_OS */
);
void noTone(int pin);

/* Simple communication protocols */
void shiftOut(int dataPin, int clockPin, int bitOrder, uint8_t val);
uint8_t shiftIn(int dataPin, int clockPin, int bitOrder);
unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout
#if !defined(ARDUINO_OS) && defined(__cplusplus)
    = 1000000
#endif /* !ARDUINO_OS */
);
unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout
#if !defined(ARDUINO_OS) && defined(__cplusplus)
    = 1000000
#endif /* !ARDUINO_OS */
);

/* Time */
unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long msecs);
void delayMicroseconds(unsigned int usecs);

/* Math */
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))
long map(long value, long fromLow, long fromHigh, long toLow, long toHigh);
double pow(double base, double exponent);
double sqrt(double x);

/* Trig */
double cos(double rad);
double tan(double rad);
double sin(double rad);

/* Characters */
static inline int isSpace(int c) {
	return (c == ' '
		|| c == '\f'
		|| c == '\n'
		|| c == '\r'
		|| c == '\t'
		|| c == '\v');
}

static inline bool isDigit(int c) {
	return (c >= '0' && c <= '9');
}

static inline bool isHexadecimalDigit(int c) {
	return ((c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F'));
}

static inline bool isUpperCase(int c) {
	return (c >= 'A' && c <= 'Z');
}

static inline bool isLowerCase(int c) {
	return (c >= 'a' && c <= 'z');
}

static inline bool isAlpha(int c) {
	return isUpperCase(c) || isLowerCase(c);
}

static inline bool isAlphaNumeric(int c) {
	return isAlpha(c) || isDigit(c);
}

static inline int toUpperCase(int c) {
	if (!isLowerCase(c))
		return c;
	return c - ('a' - 'A');
}

static inline int toLowerCase(int c) {
	if (!isUpperCase(c))
		return c;
	return c + ('a' - 'A');
}

static inline bool isAscii(int c) {
  return !(c & 0x80);
}

static inline bool isWhitespace(int c) {
  return (c == ' ') || (c == '\t');
}

static inline bool isControl(int c) {
  return (c >= 0) && (c <= 0xd);
}

static inline bool isPrintable(int c) {
  return ((c >= 0xe) && (c <= 0xff)) || (c == 0xb);
}

static inline bool isGraph(int c) {
  return !isPrintable(c);
}

static inline bool isPunct(int c) {
  return (c >= 0x21) && (c <= 0x7f) && !isAlphaNumeric(c);
}

static inline int toAscii(int c) {
  return c & 0x7f;
}

/* So Random */
long random(long min, long max);
#if !defined(ARDUINO_OS) && defined(__cplusplus)
/* Non OS-version uses overloaded version */
static inline long random(long max) {
  return random(0, max);
}
#endif
long randomSeed(unsigned long seed);

int pinToPort(int pin, ioportid_t *port, uint8_t *pad);
void arduinoIoInit(void);

/* Bits and Bytes */
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))

#define PTA(x) (0x40 | (x))
#define PTB(x) (0x40 | (32 + x))

#define A0 0x80
#define A1 0x81
#define A2 0x82
#define A3 0x83
#define A4 0x84 /* Actually a digital pin */
#define A5 0x85 /* Actually a digital pin */
#define A6 0x86 /* Temperature sensor */
#define A7 0x87 /* Voltage sensor */
#define A8 0x88 /* VDD voltage value */
#define A9 0x89 /* VCC voltage value */
#define A10 0x8a /* Audio pin */

/* Digital pins */
#define D0 0xa0
#define D1 0xa1
#define D2 0xa2
#define D3 0xa3
#define D4 0xa4
#define D5 0xa5

#define LED_BUILTIN       PTA(12)
#define LED_BUILTIN_RGB   PTA(6)

#define LED_BUILTIN_RED   PTA(5)
#define LED_BUILTIN_GREEN PTB(6)
#define AUDIO_IN          PTB(5)
#define UART_TX           PTB(2)
#define UART_RX           PTB(1)
#define SWD_CLK           PTA(0)
#define SWD_DIO           PTA(2)

/* Interrupts */

#define I2C_IRQ           8
#define SERIAL_IRQ        12
#define ADC_IRQ           15
#define PWM0_IRQ          17
#define PWM1_IRQ          18
#define LPTMR_IRQ         28
#define PORTA_IRQ         30
#define PORTB_IRQ         31

#endif /* __ARDUINO_KOSAGI_H__ */
