#include <stdint.h>
#include <stdio.h>
#include "murmur3.h"

#define MAX_TRIES 100
#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

static long long rand_val;

static void add_entropy(uint32_t val)
{
  rand_val += val;
}

// Actually generate a (somewhat) random number
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
long random_at_most(long max) {
  unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;
  int tries = 0;

  long x;
  do {
   x = real_rand();
  }
  // This is carefully written not to overflow
  while ((num_rand - defect <= (unsigned long)x) && (tries++ < MAX_TRIES));

  // Truncated division is intentional
  return x/bin_size;
}

long random(long min, long max)
{
  // Behavior undefined if min > max, but make it work anyway
  if (max < min) {
    int temp = min;
    min = max;
    max = temp;
  }

  int rand_val = random_at_most(max - min);

  add_entropy(min);
  add_entropy(max);

  return rand_val + min;
}

long randomSeed(unsigned long seed) {

  rand_val = seed;
  return 0;
}
