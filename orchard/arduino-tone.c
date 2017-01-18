#include "hal.h"
#include "Arduino.h"
#include "arduino-io.h"
#include "lptmr.h"

/* Virtual timer for disabling the tone object */
virtual_timer_t tone_timer;

void arduinoToneInit(void) {
  chVTObjectInit(&tone_timer);
}

/* Timer callback.  Called from a virtual timer to stop the tone after
 * a certain duration.
 */
static void stop_tone(void *par) {

  noTone((uint32_t)par);
}

void tone(int pin, unsigned int frequency, unsigned long duration) {

  ioportid_t port;
  uint8_t pad;

  if (pinToPort(pin, &port, &pad))
    return;

  /* Don't allow absurd frequencies */
  if (frequency > 65536)
    return;

  /* Don't let users access illegal pins.*/
  if (!canUsePin(pin))
    return;

  /* Ensure the pin is an output */
  palSetPadMode(port, pad, PAL_MODE_OUTPUT_PUSHPULL);

  /* Start up the low-power timer, which directly drives the port. */
  startLptmr(port, pad, frequency);

  /* Set up a timer callback to stop the tone. */
  if (duration)
    chVTSet(&tone_timer, MS2ST(duration), stop_tone, (void *)pin);

  return;
}

void noTone(int pin) {

  stopLptmr();

  // Set the pin LOW to prevent speaker burnout
  digitalWrite(pin, LOW);
  return;
}