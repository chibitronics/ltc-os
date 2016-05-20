#include "hal.h"

void lockSystem(void) {
  osalSysLock();
}

void lockSystemFromISR(void) {
  osalSysLockFromISR();
}

void unlockSystem(void) {
  osalSysUnlock();
}

void unlockSystemFromISR(void) {
  osalSysUnlockFromISR();
}

thread_t *createThread(void *stack, size_t stack_size, int prio,
                       void (*thr_func)(void *ptr), void *ptr) {
  return chThdCreateStatic(stack, stack_size, prio, thr_func, ptr);
}

void exitThread(msg_t msg) {
  chThdExit(msg);
}

void setTimer(virtual_timer_t *vtp, systime_t delay, vtfunc_t vtfunc, void *par) {
  chVTSet(vtp, delay, vtfunc, par);
}

void resetTimer(virtual_timer_t *vtp) {
  chVTReset(vtp);
}