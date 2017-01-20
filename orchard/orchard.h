#ifndef __ORCHARD_H__
#define __ORCHARD_H__

extern struct evt_table orchard_events;

void halt(void);

/* Version 1.8.4 */
#define LTC_OS_VERSION 0x00010804

/* LTC board, PVT, 1, no patches */
#define LTC_HW_VERSION 0x00030100

#define ORCHARD_OS_VERSION_MAJOR      1
#define ORCHARD_OS_VERSION_MINOR      0

#define serialDriver                  (&SD1)
#define stream_driver                 ((BaseSequentialStream *)serialDriver)
extern void *stream;

#define i2cDriver                     (&I2CD2)
#define accelAddr                     0x1c

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#endif

#endif /* __ORCHARD_H__ */
