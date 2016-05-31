#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "orchard-events.h"
#include "oled.h"
#include "gfx.h"

#include "analog.h"

#include "dv-oscope.h"

uint8_t speed_mode = 0;

static void draw_wave(uint16_t *samples) {
  coord_t width;
  coord_t height;
  int i;
  uint16_t min, max, offset;
  uint32_t temp;

  width = gdispGetWidth();
  height = gdispGetHeight();

  min = 0xFFFF, max = 0x0;
  for( i = 0; i < SCOPE_SAMPLE_DEPTH; i++ ) {
    if( samples[i] > max )
      max = samples[i];
    if( samples[i] < min )
      min = samples[i];
  }

  if( max == 0 )
    max = 1;  // to avoid divide by zero

  if( (((uint32_t)(max - min) * (uint32_t) height) / (uint32_t) max) < (uint32_t) height )
    offset = (height - (((uint32_t)(max - min) * (uint32_t) height) / (uint32_t) max)) / 2;
  else
    offset = 0;

  for( i = 0; i < SCOPE_SAMPLE_DEPTH; i++ ) {
    // zero-reference
    samples[i] -= min;
    
    // now scale to screen height
    temp = (uint32_t) samples[i];
    temp *= (uint32_t) height;
    temp /= max;
    samples[i] = temp;

    samples[i] += offset;
  }

  orchardGfxStart();
  gdispClear(Black);

  for( i = 0; i < width - 1; i++ ) {
    gdispDrawLine( i, height - 1 - samples[i], i+1, height - 1 - samples[i+1], White );
  }
  
  gdispFlush();
  orchardGfxEnd();
}

void updateOscopeScreen(void) {
  uint16_t *samples;
  
  // grab a reading
  samples = scopeRead(0, speed_mode);  // 1 is high, 0 is low
  
  // now display the time-averaged voltage
  draw_wave(samples);
}
