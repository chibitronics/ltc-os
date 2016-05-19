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

int _isspace(char c) {
  return (c == ' '
       || c == '\f'
       || c == '\n'
       || c == '\r'
       || c == '\t'
       || c == '\v');
}

int _isdigit(char c) {
  return (c >= '0' && c <= '9');
}

int _isxdigit(char c) {
  return ((c >= '0' && c <= '9') ||
    (c >= 'a' && c <= 'f') ||
    (c >= 'A' && c <= 'F'));
}

int _isupper(char c) {
  return (c >= 'A' && c <= 'Z');
}

int _islower(char c) {
  return (c >= 'a' && c <= 'z');
}

int _isalpha(char c) {
  return _isupper(c) || _islower(c);
}

int _isalnum(char c) {
  return _isalpha(c) || _isdigit(c);
}

int _toupper(char c) {
  if (!_islower(c))
    return c;
  return c - ('a' - 'A');
}

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {

  unsigned long result = 0, value;

  if (*cp == '0') {
    cp++;
    if ((*cp == 'x') && _isxdigit(cp[1])) {
      base = 16;
      cp++;
    }
    if (!base) {
      base = 8;
    }
  }
  if (!base) {
    base = 10;
  }
  while (_isxdigit(*cp) && (value = _isdigit(*cp) ? *cp-'0' : (_islower(*cp)
      ? _toupper(*cp) : *cp)-'A'+10) < base) {
    result = result*base + value;
    cp++;
  }
  if (endp)
    *endp = (char *)cp;
  return result;
}

long simple_strtol(const char *cp, char **endp, unsigned int base) {
  if (*cp=='-')
    return -simple_strtoul(cp+1,endp,base);
  return simple_strtoul(cp,endp,base);
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

void *memset_aligned(void *dst0, int val, size_t length) {
  uint32_t *ptr = dst0;
  length /= sizeof(*ptr);

  while (length--)
    *ptr++ = val;
  return dst0;
}

void *memset(void *dst0, int val, size_t length) {
  uint8_t *ptr = dst0;
  while (length--)
    *ptr++ = val;
  return dst0;
}

void *memclr(void *dst0, size_t length) {
  return memset(dst0, 0, length);
}

void *memmove(void *dest, const void *src, size_t n) {
  return memcpy(dest, src, n);
}

void *memcpy_aligned(void *dest, const void *src, size_t n) {
  size_t i;
  uint32_t *d = dest;
  const uint32_t *s = src;
  n /= sizeof(*d);

  for (i = 0; i < n; i++)
    d[i] = s[i];

  return d;
}

void *memmove_aligned(void *dest, const void *src, size_t n) {
  return memcpy_aligned(dest, src, n);
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

char *strcpy(char *dest, const char *src) {
  size_t i;

  for (i = 0; src[i] != '\0'; i++)
    dest[i] = src[i];
  dest[i] = '\0';

  return dest;
}

char *strchr(const char *s, int c) {
  while (*s && (*s != c))
    s++;
  if (*s == c)
    return (char *)s;
  return NULL;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (; *s1 == *s2; s1++,s2++)
    if ( *s1 == '\0' || --n <=0)
      return 0;
  return *s1 - *s2;
}

int strcmp(const char *s1, const char *s2) {
  return strncmp(s1, s2, ~0);
}

size_t strlen(const char *s) {
  int i = 0;
  while(s[i])
    i++;
  return i;
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
