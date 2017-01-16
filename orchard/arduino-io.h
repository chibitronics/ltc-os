#ifndef _ARDUINO_IO_H
#define _ARDUINO_IO_H

int canonicalizePin(int pin);
int pinToPort(int pin, ioportid_t *port, uint8_t *pad);
int canUsePin(int pin);

#endif /* _ARDUINO_IO_H */