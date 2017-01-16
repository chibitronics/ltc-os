#include "Arduino.h"
#include "murmur3.h"

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static long long rand_val;

static void add_entropy(uint32_t val)
{
  rand_val += val;
}

static uint32_t real_rand(void)
{
  uint32_t val;

  // Run it through murmurhash, since we have it already
  MurmurHash3_x86_32(&rand_val, sizeof(rand_val), 0, &val);

  // Save this value for adding to entropy next time
  add_entropy(val);

  return val;
}

// From Stack Exchange:
// http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
#define MAX_TRIES 100
#ifndef RAND_MAX
#define RAND_MAX 4294967295
#endif
long random(long min, long max)
{
  int r;
  const int range = 1 + max - min;
  const int buckets = RAND_MAX / range;
  const int limit = buckets * range;
  int tries = 0;

  // Undefined if min > max, but make it work anyway
  if (max < min) {
    int temp = min;
    min = max;
    max = temp;
  }

  /* Create equal size buckets all in a row, then fire randomly towards
   * the buckets until you land in one of them. All buckets are equally
   * likely. If you land off the end of the line of buckets, try again. */
  do {
    r = real_rand();
  } while ((r >= limit) && (tries++ < MAX_TRIES));

  add_entropy(min);
  add_entropy(max);

  return min + (r / buckets);
}

long randomSeed(unsigned long seed) {

  rand_val = seed;
  return 0;
}
