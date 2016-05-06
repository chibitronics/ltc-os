#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>

struct app_header {

  /* 0x00 */
  uint32_t *data_load_start;  /* Start of data in ROM */
  uint32_t *data_start;       /* Start of data load address in RAM */
  uint32_t *data_end;         /* End of data load address in RAM */
  uint32_t *bss_start;        /* Start of BSS in RAM */

  /* 0x10 */
  uint32_t *bss_end;          /* End of BSS in RAM */
  void (*entry)(void);        /* Address to jump to */
  uint32_t magic;             /* 32-bit signature, defined below */
  uint32_t version;           /* BCD version number */

  /* 0x30 */
  uint32_t *const_start;      /* Start of C++ constructors */
  uint32_t *const_end;        /* Start of C++ constructors */
  uint32_t reserved2;
  uint32_t reserved3;
} __attribute__((__packed__));

#define APP_MAGIC 0xd3fbf67a
#define APP_VERSION 0x00000100 /* 0.0.1.0 */

__attribute__((noreturn)) void Run_App(struct app_header *app);

#endif
