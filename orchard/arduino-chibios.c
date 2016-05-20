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

msg_t suspendThread(thread_reference_t *trp) {
  return chThdSuspendS(trp);
}

msg_t suspendThreadTimeout(thread_reference_t *trp, systime_t timeout) {
  return chThdSuspendTimeoutS(trp, timeout);
}

void resumeThread(thread_reference_t *trp, msg_t msg) {
  chThdResume(trp, msg);
}

void yieldThread(void) {
  chThdYield();
}

void threadSleep(systime_t time) {
  chThdSleep(time);
}

void threadSleepUntil(systime_t time) {
  chThdSleepUntil(time);
}

msg_t waitThread(thread_t *tp) {
  return chThdWait(tp);
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

uint32_t getSyscallABI(void) {
  return 1;
}