#include "hal.h"
#include "esplanade_app.h"
#include "orchard.h"

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


struct vt_callback {
  vtfunc_t func;
  void *param;
};

static struct vt_callback vtcallbacks[4];
static struct vt_callback scheduled_callbacks[4];
static uint8_t num_scheduled_callbacks = 0;

int runCallbacks(void) {

  unsigned int i;

  if (!num_scheduled_callbacks)
    return 0;

  for (i = 0; i < ARRAY_SIZE(scheduled_callbacks); i++) {
    if (scheduled_callbacks[i].func) {
      scheduled_callbacks[i].func(scheduled_callbacks[i].param);
      scheduled_callbacks[i].func = NULL;
      num_scheduled_callbacks--;
    }
  }

  return 0;
}


static void post_timer_callback_to_arduino_thread(void *par) {

  unsigned int i;
  struct vt_callback *vtcallback = par;
  void *param = vtcallback->param;
  vtfunc_t func = vtcallback->func;

  /* Nullify these parameters so they can be requeued. */
  vtcallback->param = NULL;
  vtcallback->func  = NULL;

  for (i = 0; i < ARRAY_SIZE(scheduled_callbacks); i++) {
    if (!scheduled_callbacks[i].func) {
      scheduled_callbacks[i].func = func;
      scheduled_callbacks[i].param = param;
      num_scheduled_callbacks++;
      return;
    }
  }
}

int setTimer(virtual_timer_t *vtp,
             systime_t delay,
             vtfunc_t vtfunc,
             void *par) {
  unsigned int i;
  int found = 0;
  for (i = 0; i < ARRAY_SIZE(vtcallbacks); i++) {
    if (!vtcallbacks[i].func) {
      vtcallbacks[i].func = vtfunc;
      vtcallbacks[i].param = par;
      found = 1;
      break;
    }
  }

  /* We couldn't find a free slot for our callback array.  Sad times. */
  if (!found)
    return -1;

  chVTSet(vtp, delay, post_timer_callback_to_arduino_thread, &vtcallbacks[i]);

  return 0;
}

void resetTimer(virtual_timer_t *vtp) {
  chVTReset(vtp);
}

uint32_t getSyscallABI(void) {
  return 1;
}
