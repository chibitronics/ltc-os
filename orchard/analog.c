#include "ch.h"
#include "hal.h"
#include "adc.h"

#include "orchard.h"
#include "orchard-events.h"
#include "analog.h"

#include "chbsem.h"

static adcsample_t scope_sample[SCOPE_SAMPLE_DEPTH];

uint16_t *scopeRead(int adc_num, int speed_mode) {
  msg_t result;

  ADCConversionGroup arduinogrp = {
    0, // circular buffer mode? no.
    1, // just one channel
    NULL,  // callback
    NULL,  // error callback
    (1 << adc_num),
    // CFG1 register
    // SYSCLK = 48MHz.
    // BUSCLK = SYSCLK / 4 = 12MHz
    // ADCCLK = SYSCLK / 2 / 1 = 6 MHz

    // ADLPC = 0 (normal power)
    // ADLSMP = 0 (short sample time)
    // ADHSC = 0 (normal conversion sequence)
    // -> 5 ADCK cycles + 5 bus clock cycles = SFCadder
    // 20 ADCK cycles per sample
    
    speed_mode ?
    ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_2) |
    ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
    ADCx_CFG1_MODE(ADCx_CFG1_MODE_8_OR_9_BITS)  // 8 bits per sample
    :
    ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_2) |
    ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
    ADCx_CFG1_MODE(ADCx_CFG1_MODE_12_OR_13_BITS),  // 12 bits per sample

    // SC3 register
    speed_mode ? 
    0 // 1 sample
    :
    ADCx_SC3_AVGE |
    ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES) // 32 sample average

    //      48 MHz sysclk
    // /2   24 MHz busclk
    // /2   12 MHz after prescaler
    // /2   6 MHz after adiv
    // /17  353ksps after base sample time @ 12 bps
    // /1   353ksps with no averaging
  };

  result = adcConvert(&ADCD1,
                     &arduinogrp,
                     scope_sample,
                     SCOPE_SAMPLE_DEPTH);
  if (result)
    return NULL;

  return (uint16_t *)scope_sample;
}

void analogStart() {
  
}

uint32_t analogRead(int adc_num) {
  msg_t result;
  adcsample_t sample;

  ADCConversionGroup arduinogrp = {
    0, // circular buffer mode? no.
    1, // just one channel
    NULL,  // callback
    NULL,  // error callback
    (1 << adc_num),
    // CFG1 register
    // SYSCLK = 48MHz.
    // BUSCLK = SYSCLK / 4 = 12MHz
    // ADCCLK = SYSCLK / 2 / 1 = 6 MHz

    // ADLPC = 0 (normal power)
    // ADLSMP = 0 (short sample time)
    // ADHSC = 0 (normal conversion sequence)
    // -> 5 ADCK cycles + 5 bus clock cycles = SFCadder
    // 20 ADCK cycles per sample

    ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_2) |
    ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
    ADCx_CFG1_MODE(ADCx_CFG1_MODE_12_OR_13_BITS),  // 12 bits per sample

    // SC3 register
    ADCx_SC3_AVGE |
    ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES) // 32 sample average

    //      48 MHz sysclk
    // /2   24 MHz busclk
    // /2   12 MHz after prescaler
    // /2   6 MHz after adiv
    // /20  300ksps after base sample time @ 12 bps
    // /32  9.375ksps after averaging by factor of 32
  };

  result = adcConvert(&ADCD1,
                     &arduinogrp,
                     &sample,
                     1);
  if (result)
    sample = 0;

  return (uint32_t) sample;
}

