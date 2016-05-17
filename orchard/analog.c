#include "hal.h"
#include "adc.h"
#include "esplanade_os.h"

#include "orchard.h"
#include "analog.h"
#include "demod.h"

bl_symbol(static adcsample_t mic_sample[MIC_SAMPLE_DEPTH]);

extern volatile uint8_t dataReadyFlag;
extern volatile adcsample_t *bufloc;
extern size_t buf_n;

static void analog_fast_isr(void) {
  ADCDriver *adcp = &ADCD1;

  /* Read the sample into the buffer */
  adcp->samples[adcp->current_index++] = adcp->adc->RA;

  /*  At the end of the buffer then we may be finished */
  if (adcp->current_index == adcp->number_of_samples) {
    /* Invokes the callback passing the 2nd half of the buffer.*/
    size_t half = adcp->depth / 2;
    size_t half_index = half * adcp->grpp->num_channels;
    adcp->grpp->end_cb(adcp, adcp->samples + half_index, half);
    adcp->current_index = 0;
  }

  if( adcp->current_index == (adcp->number_of_samples / 2) ) {
    _adc_isr_half_code(&ADCD1);
  }
}

static void adc_mic_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)n;

#if !DEMOD_DEBUG  // select this path for "normal" orchard operation
  chSysLockFromISR();
  chEvtBroadcastI(&adc_mic_event);
  chSysUnlockFromISR();
#else  // single-thread operation
  dataReadyFlag = 1;
  bufloc = buffer;
  buf_n = n;
#endif
}


/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 */
static const ADCConversionGroup adcgrpmic = {
  true, // circular buffer mode
  1, // just one channel
  adc_mic_end_cb,  // callback
  NULL,  // error callback
  ADC_DADP0,  // microphone input channel
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
  ADCx_SC3_ADCO |   // continuous conversions
  ADCx_SC3_AVGE |
  ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_4_SAMPLES) // 4 sample average
  
  //      48 MHz sysclk
  // /2   24 MHz busclk
  // /2   12 MHz after prescaler
  // /2   6 MHz after adiv
  // /20  300ksps after base sample time @ 12 bps
  // /4   75ksps after averaging by factor of 4
};

void analogUpdateMic(void) {
  //  adcAcquireBus(&ADCD1);
  //  adcConvert(&ADCD1, &adcgrpmic, mic_sample, MIC_SAMPLE_DEPTH);
  adcStartConversion(&ADCD1, &adcgrpmic, mic_sample, MIC_SAMPLE_DEPTH);
  //  adcReleaseBus(&ADCD1);
}

void analogStart(void) {
  adcFastISR = analog_fast_isr;
}

void analogStop(void) {
  adcStopConversion(&ADCD1);
  adcStop(&ADCD1);
  adcFastISR = NULL;
}
