/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <string.h>

#include "hal.h"
#include "esplanade_app.h"

__attribute__((aligned(4)))
extern struct app_header _app_header;
memory_heap_t app_heap;
thread_t *esplanadeThread;

extern systime_t app_start_time;

static const ADCConfig adccfg1 = {
    /* Perform initial calibration */
    true
};

void ArduinoInit(void) {

  adcStart(&ADCD1, &adccfg1);
}

void *malloc(size_t size) {
  return chHeapAlloc(&app_heap, size);
}

void free(void *ptr) {
  if (ptr)
    chHeapFree(ptr);
}

/* http://www.chibios.com/forum/viewtopic.php?t=1314&p=22848#p22848 */
void *realloc(void *addr, size_t size)
{
   union heap_header *hp;
   size_t prev_size, new_size;

   void *ptr;

   if (addr == NULL) {
      return chHeapAlloc(&app_heap, size);
   }

   /* previous allocated segment is preceded by an heap_header */
   hp = addr - sizeof(union heap_header);
   prev_size = hp->h.size; /* size is always multiple of 8 */

   /* check new size memory alignment */
   if(size % 8 == 0) {
      new_size = size;
   }
   else {
      new_size = ((int) (size / 8)) * 8 + 8;
   }

   if(prev_size >= new_size) {
      return addr;
   }

   ptr = chHeapAlloc(&app_heap, size);
   if(ptr == NULL) {
      return NULL;
   }

   memcpy(ptr, addr, prev_size);

   chHeapFree(addr);

   return ptr;
} /* realloc */

static THD_FUNCTION(app_thread, arg) {

  (void)arg;
  void (**func)(void);

  chRegSetThreadName("user code");

  /* Set up the heap */
  void *heap_start = _app_header.heap_start + sizeof(thread_t);
  size_t heap_size = _app_header.heap_end - heap_start - sizeof(thread_t);

  chHeapObjectInit(&app_heap, (void *)MEM_ALIGN_NEXT(heap_start), MEM_ALIGN_NEXT((size_t)heap_size));

  ArduinoInit();

  /* Call each of the C++ constructors pointed to by the header */
  func = _app_header.const_start;
  while (func != _app_header.const_end) {
    (*func)();
    func++;
  }

  /* Enter the main Arduino application */
  _app_header.entry();
}

int appIsValid(void) {
  if ((_app_header.magic == APP_MAGIC)
   && (_app_header.version == APP_VERSION))
    return 1;
  return 0;
}

thread_t *chBootToApp(void) {

  int irq_num;

  arduinoIoInit();

  app_start_time = chVTGetSystemTimeX();

  memset(_app_header.bss_start, 0,
         _app_header.bss_end - _app_header.bss_start);
  memcpy(_app_header.data_start, _app_header.data_load_start,
         _app_header.data_end - _app_header.data_start);

  /* In order to allow syscalls to happen during ISRs, prioritize the
   * "svc" handler above all others.
   * Make everything else have the lowest priority.  If a user wants to
   * increase priority, they can adjust priorities themselves.
   */
  NVIC_SetPriority(SVCall_IRQn, 0);
  NVIC_SetPriority(PendSV_IRQn, 3);
  NVIC_SetPriority(SysTick_IRQn, 3);
  for (irq_num = 0; irq_num < CORTEX_NUM_VECTORS; irq_num++)
    if (NVIC_GetPriority((IRQn_Type)irq_num) < 3)
      NVIC_SetPriority((IRQn_Type)irq_num, 3);

  esplanadeThread = chThdCreateStatic(_app_header.heap_start,
                           (_app_header.heap_end - _app_header.heap_start) & ~7,
                           NORMALPRIO,
                           app_thread,
                           NULL);

  return esplanadeThread;
}

int avoid_isr_lock;

int avoidIsrLock(int val)
{
  avoid_isr_lock = val;
  return 0;
}
