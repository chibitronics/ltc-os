#ifndef __APP_H__
#define __APP_H__

#include <stdint.h>

#if 0
#define PROG_STATR_ON   GPIOA->PCOR = (1 << 5)
#define PROG_STATR_OFF  GPIOA->PSOR = (1 << 5)
#define PROG_STATG_ON   GPIOB->PCOR = (1 << 6)
#define PROG_STATG_OFF  GPIOB->PSOR = (1 << 6)
#define RESET_LEVEL     GPIOB->PDIR &= (1 << 7)
#else
extern void ledUpdate(uint32_t num_leds, void *pixels, uint32_t mask,
                          uint32_t set_addr, uint32_t clr_addr);

//#define FGPIOA_PDOR 0xF8000000 /* Port Data Output Register */
//#define FGPIOA_PSOR 0xF8000004 /* Port Set Output Register */
//#define FGPIOA_PCOR 0xF8000008 /* Port Clear Output Register */

#if 0
#define PROG_STATR_ON   GPIOA->PCOR = (1 << 5)
#define PROG_STATR_OFF  GPIOA->PSOR = (1 << 5)
#define PROG_STATG_ON   GPIOB->PCOR = (1 << 6)
#define PROG_STATG_OFF  GPIOB->PSOR = (1 << 6)
#else
#define PROG_STATR_ON   { uint32_t pixel = 0x1f001f; ledUpdate(1, &pixel, (1 << 6), 0xF8000004,  0xF8000008); }
#define PROG_STATR_OFF  { uint32_t pixel = 0x00001f; ledUpdate(1, &pixel, (1 << 6), 0xF8000004,  0xF8000008); }
#define PROG_STATG_ON   { uint32_t pixel = 0x001f1f; ledUpdate(1, &pixel, (1 << 6), 0xF8000004,  0xF8000008); }
#define PROG_STATG_OFF  { uint32_t pixel = 0x00001f; ledUpdate(1, &pixel, (1 << 6), 0xF8000004,  0xF8000008); }
#endif

#define RESET_LEVEL     GPIOB->PDIR &= (1 << 4)
#endif

/* This function gets called on an update error, or on HardFault */
__attribute__((noreturn)) void errorCondition(void);

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
extern memory_heap_t app_heap;

#endif
