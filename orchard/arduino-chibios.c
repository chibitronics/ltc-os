#include "hal.h"
#include "esplanade_app.h"
#include "orchard.h"

void (*sysTickHook)(void);

void doSysTick(void) {
  if (sysTickHook)
    sysTickHook();
}

void hookSysTick(void (*newHook)(void)) {
  sysTickHook = newHook;
}

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

msg_t suspendThreadS(thread_reference_t *trp) {
  msg_t ret;

  ret = chThdSuspendS(trp);
  return ret;
}

msg_t suspendThread(thread_reference_t *trp) {
  msg_t ret;

  osalSysLock();
  ret = chThdSuspendS(trp);
  osalSysUnlock();
  return ret;
}

msg_t suspendThreadTimeoutS(thread_reference_t *trp, systime_t timeout) {
  msg_t ret;

  ret = chThdSuspendTimeoutS(trp, timeout);
  return ret;
}

msg_t suspendThreadTimeout(thread_reference_t *trp, systime_t timeout) {
  msg_t ret;

  osalSysLock();
  ret = chThdSuspendTimeoutS(trp, timeout);
  osalSysUnlock();
  return ret;
}

void resumeThread(thread_reference_t *trp, msg_t msg) {
  chThdResume(trp, msg);
}

void resumeThreadI(thread_reference_t *trp, msg_t msg) {
  chThdResumeI(trp, msg);
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

static struct vt_callback vtcallbacks[2];
static struct vt_callback scheduled_callbacks[2];
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

uint32_t getSyscallAddr(uint32_t sysCallNum) {
  extern uint16_t SysCall_Table;
  uint16_t *calls = &SysCall_Table;
  return calls[sysCallNum] | 1;
}

void setSerialSpeed(uint32_t speed) {
  SerialConfig serialConfig = {
    speed,
  };

  sdStop(&SD1);
  sdStart(&SD1, &serialConfig);
}

void mutexInit(mutex_t *mp) {
  chMtxObjectInit(mp);
}

void mutexLock(mutex_t *mp) {
  chMtxLock(mp);
}

bool mutexTryLock(mutex_t *mp) {
  return chMtxTryLock(mp);
}

void mutexUnlock(mutex_t *mp) {
  chMtxUnlock(mp);
}

void setThreadName(const char *name) {
  chRegSetThreadName(name);
}
