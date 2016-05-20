#include "Arduino.h"
#include "murmur3.h"

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
