#ifndef __ORCHARD_H__
#define __ORCHARD_H__

/* Version 1.9.1 */
#define LTC_OS_VERSION 0x00010901

#define serialDriver                  (&SD1)
#define stream_driver                 ((BaseSequentialStream *)serialDriver)
extern void *stream;

#define i2cDriver                     (&I2CD2)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#endif

#endif /* __ORCHARD_H__ */
