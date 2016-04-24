#include "ch.h"
#include "hal.h"
#include "adc.h"

#include "orchard.h"
#include "orchard-events.h"
#include "analog.h"
#include "demod.h"

#include "chbsem.h"

static adcsample_t mic_sample[MIC_SAMPLE_DEPTH];

#define ADC_GRPCELCIUS_NUM_CHANNELS   2
#define ADC_GRPCELCIUS_BUF_DEPTH      1
static int32_t celcius;
static adcsample_t celcius_samples[ADC_GRPCELCIUS_NUM_CHANNELS * ADC_GRPCELCIUS_BUF_DEPTH];

static void adc_temperature_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)n;

  /*
   * The bandgap value represents the ADC reading for 1.0V
   */
  uint16_t sensor = buffer[0];
  uint16_t bandgap = buffer[1];

  /*
   * The v25 value is the voltage reading at 25C, it comes from the ADC
   * electricals table in the processor manual. V25 is in millivolts.
   */
  int32_t v25 = 716;

  /*
   * The m value is slope of the temperature sensor values, again from
   * the ADC electricals table in the processor manual.
   * M in microvolts per degree.
   */
  int32_t m = 1620;

  /*
   * Divide the temperature sensor reading by the bandgap to get
   * the voltage for the ambient temperature in millivolts.
   */
  int32_t vamb = (sensor * 1000) / bandgap;

  /*
   * This formula comes from the reference manual.
   * Temperature is in millidegrees C.
   */
  int32_t delta = (((vamb - v25) * 1000000) / m);
  celcius = 25000 - delta;

  chSysLockFromISR();
  chEvtBroadcastI(&adc_celcius_event);
  chSysUnlockFromISR();
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 * Note: this comment above is from chibiOS sample code. I don't actually get
 *  what they mean by "8 samples of 1 channel" because that doesn't look like
 *  what's happening. 
 */
static const ADCConversionGroup adcgrpcelcius = {
  false,
  ADC_GRPCELCIUS_NUM_CHANNELS,
  adc_temperature_end_cb,
  NULL,
  ADC_TEMP_SENSOR | ADC_BANDGAP,
  /* CFG1 Regiser - ADCCLK = SYSCLK / 16, 16 bits per sample */
  ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
  ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
  ADCx_CFG1_MODE(ADCx_CFG1_MODE_16_BITS),
  /* SC3 Register - Average 32 readings per sample */
  ADCx_SC3_AVGE |
  ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES)
};

void analogUpdateTemperature(void) {
  adcAcquireBus(&ADCD1);
  adcStartConversion(&ADCD1, &adcgrpcelcius, celcius_samples, ADC_GRPCELCIUS_BUF_DEPTH);
  adcReleaseBus(&ADCD1);
}

int32_t analogReadTemperature() {
  return celcius;
}

extern volatile uint8_t dataReadyFlag;
extern volatile adcsample_t *bufloc;
extern size_t buf_n;

static void adc_mic_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)n;

  uint16_t i;

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
  adcAcquireBus(&ADCD1);
  //  adcConvert(&ADCD1, &adcgrpmic, mic_sample, MIC_SAMPLE_DEPTH);
  adcStartConversion(&ADCD1, &adcgrpmic, mic_sample, MIC_SAMPLE_DEPTH);
  adcReleaseBus(&ADCD1);
}

void analogStart() {
  
}


