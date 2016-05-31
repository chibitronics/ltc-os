#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "orchard-events.h"
#include "oled.h"
#include "gfx.h"

#include "analog.h"

#include "dv-volts.h"

/*
  Time-averaging isn't for noise. It's to make the voltage converge on a final value
  that's in a human-perceivable time scale...the digital equivalent of a needle moving, if you will.
 */
#define VOLT_BUF_LEN  8
static uint32_t volt_buf[VOLT_BUF_LEN] = {0,0,0,0, 0,0,0,0};
static uint8_t volt_ptr = 0;

static void draw_volts(uint32_t millivolts) {
  char vstr[7];
  coord_t width;
  coord_t height;
  coord_t font_height;
  font_t font;

  chsnprintf(vstr, sizeof(vstr), "%d.%03d", millivolts / 1000, millivolts % 1000 );

  orchardGfxStart();
  width = gdispGetWidth();
  height = gdispGetHeight();

  font = gdispOpenFont("LargeNumbers");
  gdispClear(Black);
  gdispDrawStringBox(0, 14, width, gdispGetFontMetric(font, fontHeight),
                     vstr, font, White, justifyCenter);
  gdispCloseFont(font);

  font = gdispOpenFont("DejaVuSans12");
  font_height = gdispGetFontMetric(font, fontHeight);
  gdispDrawStringBox(0, height - font_height * 2, width, gdispGetFontMetric(font, fontHeight),
                     "volts", font, White, justifyCenter);

  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

void updateVoltsScreen(void) {
  uint32_t voltsVal;
  uint32_t calVal;
  int i;
  uint32_t voltAvg = 0;

  // first update the sample buffer
  calVal = analogRead(27);  // channel 27 is cal = 1.0V
  // value returned is # of LSB = 1.0V
  voltsVal = analogRead(0) * 1000;
  
  volt_buf[volt_ptr] = voltsVal / calVal;  // this is volts in mV
  volt_ptr++;
  volt_ptr %= VOLT_BUF_LEN;

  // now compute the average
  for( i = 0; i < VOLT_BUF_LEN; i++ ) {
    voltAvg += volt_buf[i];
  }
  voltAvg /= VOLT_BUF_LEN;
  
  // now display the time-averaged voltage
  draw_volts(voltAvg);
}
