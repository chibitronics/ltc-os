#include "hal.h"
#include "adc.h"
#include "esplanade_os.h"

#include "orchard.h"
#include "esplanade_analog.h"
#include "esplanade_demod.h"

bl_symbol_bss(static adcsample_t mic_sample[MIC_SAMPLE_DEPTH]);
bl_symbol_bss(static uint32_t adc_current_index);
bl_symbol_bss(static const volatile uint32_t *adc_ra);

extern volatile uint8_t dataReadyFlag;
extern volatile adcsample_t *bufloc;
extern size_t buf_n;

static void analog_fast_isr(void) {

  /* Read the sample into the buffer */
  mic_sample[adc_current_index++] = *adc_ra;//adcp->adc->RA;

  /* If buffer 2 has filled, deliver samples and move to buffer 1 */
  if (adc_current_index == MIC_SAMPLE_DEPTH) {
    /* Invokes the callback passing the 2nd half of the buffer.*/
    size_t half = MIC_SAMPLE_DEPTH / 2;
    size_t half_index = half * 1;

    dataReadyFlag = 1;
    bufloc = mic_sample + half_index;
    buf_n = half;
    adc_current_index = 0;
  }

  /* If buffer 1 has filled up, deliver the samples and continue to buffer 2 */
  else if (adc_current_index == (MIC_SAMPLE_DEPTH / 2)) {
    dataReadyFlag = 1;
    bufloc = mic_sample;
    buf_n = MIC_SAMPLE_DEPTH / 2;
  }
}

static void adc_mic_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)buffer;
  (void)n;

  /* This function is unused, as the callback isn't used in "fast" mode. */
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
  adcStartConversion(&ADCD1, &adcgrpmic, mic_sample, MIC_SAMPLE_DEPTH);
}

void analogStart(void) {
  adcFastISR = analog_fast_isr;
  adc_ra = &ADCD1.adc->RA;
}

void analogStop(void) {
  adcStopConversion(&ADCD1);
  adcStop(&ADCD1);
  adcFastISR = NULL;
}
