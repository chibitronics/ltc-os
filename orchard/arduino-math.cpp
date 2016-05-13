#include "Arduino.h"
#include "murmur3.h"

long map(long value, long fromLow, long fromHigh, long toLow, long toHigh) {
  return 0;
}

static long long rand_val;
#warning "Value exceeds max, min"
long random(long min, long max) {
  int32_t val;

  MurmurHash3_x86_32(&rand_val, sizeof(rand_val), 0, &val);

  rand_val += val;

  return val;
}

long randomSeed(unsigned long seed) {

  rand_val = seed;
  return 0;
}
