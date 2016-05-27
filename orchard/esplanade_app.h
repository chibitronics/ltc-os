#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>

struct app_header {

  /* 0x00 */
  void *data_load_start;      /* Start of data in ROM */
  void *data_start;           /* Start of data load address in RAM */
  void *data_end;             /* End of data load address in RAM */
  void *bss_start;            /* Start of BSS in RAM */

  /* 0x10 */
  void *bss_end;              /* End of BSS in RAM */
  void (*entry)(void);        /* Address to jump to */
  uint32_t magic;             /* 32-bit signature, defined below */
  uint32_t version;           /* BCD version number */

  /* 0x30 */
  void (**const_start)(void); /* Start of C++ constructors */
  void (**const_end)(void);   /* Start of C++ constructors */
  void *heap_start;           /* Start of heap */
  void *heap_end;             /* End of heap */
} __attribute__((packed, aligned(4)));

#define APP_MAGIC 0xd3fbf67a
#define APP_VERSION 0x00000100 /* 0.0.1.0 */

thread_t *chBootToApp(void);
int appIsValid(void);

extern thread_t *esplanadeThread;

#endif
