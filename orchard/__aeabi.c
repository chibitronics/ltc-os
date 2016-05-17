#if defined(__clang__)
// clang interprets -fno-builtin more loosely than you might expect,
// and thinks it's okay to still substitute builtins as long as they're
// named __aeabi_* rather than __builtin_*, which causes infinite
// recursion if we have the fortified memcpy visible in this file.
#undef _FORTIFY_SOURCE
#endif

#include <string.h>
#include <stdint.h>
#include <stdarg.h>

void do_nothing(void) {
  return;
}

typedef uint64_t stkalign_t;
uint8_t *os_heap_start = 0;
uint8_t *os_heap_end = 0;

#define MEM_ALIGN_SIZE      sizeof(stkalign_t)
#define MEM_ALIGN_MASK      (MEM_ALIGN_SIZE - 1U)
#define MEM_ALIGN_PREV(p)   ((size_t)(p) & ~MEM_ALIGN_MASK)
#define MEM_ALIGN_NEXT(p)   MEM_ALIGN_PREV((size_t)(p) + MEM_ALIGN_MASK)
void *_sbrk(size_t increment) {
  void *p;
  if (!os_heap_start && !os_heap_end)
    return NULL;

  increment = MEM_ALIGN_NEXT(increment);
  /*lint -save -e9033 [10.8] The cast is safe.*/
  if ((size_t)(os_heap_end - os_heap_start) < increment) {
    /*lint -restore*/
    return NULL;
  }
  p = os_heap_start;
  os_heap_start += increment;

  return p;
}

extern char *ch_ltoa(char *p, long num, unsigned radix);

char *itoa(long val, char *s, int radix) {
  return ch_ltoa(s, val, radix);
}

char *ltoa(long val, char *s, int radix) {
  return ch_ltoa(s, val, radix);
}

char *ultoa(unsigned long val, char *s, int radix) {
  return ch_ltoa(s, val, radix);
}

char *utoa(unsigned long val, char *s, int radix) {
  return ch_ltoa(s, val, radix);
}

void *memset (void *dst0, int val, size_t length) {
  uint8_t *ptr = dst0;
  while(length--)
    *ptr++ = val;
  return dst0;
}

void *memmove(void *dest, const void *src, size_t n) {
  return memcpy(dest, src, n);
}

void *memcpy(void *dest, const void *src, size_t n) {
  size_t i;
  uint8_t *d = dest;
  const uint8_t *s = src;

  for (i = 0; i < n; i++)
    d[i] = s[i];

  return d;
}

/* Taken from the manpage */
char *strncpy(char *dest, const char *src, size_t n) {
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++)
    dest[i] = src[i];
  for ( ; i < n; i++)
    dest[i] = '\0';

  return dest;
}

size_t strlen(const char *s) {
  int i = 0;
  while(s[i++]);
  return i;
}

int printf(const char *fmt, ...) {
  va_list ap;
  int formatted_bytes;
  extern void *stream;

  va_start(ap, fmt);
  formatted_bytes = chvprintf(stream, fmt, ap);
  va_end(ap);

  return formatted_bytes;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  unsigned char u1, u2;

  for ( ; n-- ; s1++, s2++) {
    u1 = * (unsigned char *) s1;
    u2 = * (unsigned char *) s2;
    if ( u1 != u2)
      return (u1-u2);
  }
  return 0;
}
