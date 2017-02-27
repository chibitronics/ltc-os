#include <string.h>
#include "esplanade_os.h"

#include "hal.h"
#include "pal.h"

#include "esplanade_analog.h"
#include "esplanade_demod.h"
#include "dsptables.h"

bl_symbol_bss(demod_sample_t dm_buf[DMBUF_DEPTH]);
bl_symbol_bss(static FSK_demod_state fsk_state);

/* Accelerated versions for 32-bit-aligned operations. */
extern void *memset_aligned(void *dst, int val, size_t length);
extern void *memmove_aligned(void *dst, const void *src, size_t length);

#if 1
int32_t FSK_core(demod_sample_t *b) {
  int32_t j;
  int32_t corrs[4] = {};
  int32_t sum = 0;

  // this optimization reduces time to 122us
  // with -O2 we get to 111us
  // with -O3 we get to 68.8us; out of 1.770ms, 1.155ms is used for processing ~65% CPU load

  /* non coherent FSK demodulation - not optimal, but it seems
     very difficult to do another way */
  //    corr = dsp_dot_prod(fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size,
  //			fsk_const.filter_hi_i, fsk_const.filter_size, 0);
  for( j = 0; j < fsk_const.filter_size; j++ ) {
    corrs[0] += b[j] * fsk_const.filter_hi_i[j];
    corrs[1] += b[j] * fsk_const.filter_hi_q[j];
    corrs[2] += b[j] * fsk_const.filter_lo_i[j];
    corrs[3] += b[j] * fsk_const.filter_lo_q[j];
  }

  corrs[0] >>= COS_BITS;
  corrs[1] >>= COS_BITS;
  corrs[2] >>= COS_BITS;
  corrs[3] >>= COS_BITS;

  sum += (corrs[0] * corrs[0]);
  sum += (corrs[1] * corrs[1]);
  sum -= (corrs[2] * corrs[2]);
  sum -= (corrs[3] * corrs[3]);

  return sum;
}
#else
int32_t FSK_core(demod_sample_t *b) {
  int32_t j;
  int32_t corr;
  int32_t sum;

  // this optimization reduces time to 122us
  // with -O2 we get to 111us
  // with -O3 we get to 68.8us; out of 1.770ms, 1.155ms is used for processing ~65% CPU load

  /* non coherent FSK demodulation - not optimal, but it seems
     very difficult to do another way */
  //    corr = dsp_dot_prod(fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size,
  //			fsk_const.filter_hi_i, fsk_const.filter_size, 0);
  corr = 0;
  for( j = 0; j < fsk_const.filter_size; j++ ) {
    corr += b[j] * fsk_const.filter_hi_i[j];
  }
  corr = corr >> COS_BITS;
  sum = corr * corr;

  //    corr = dsp_dot_prod(fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size,
  //			fsk_const.filter_hi_q, fsk_const.filter_size, 0);
  corr = 0;
  for( j = 0; j < fsk_const.filter_size; j++ ) {
    corr += b[j] * fsk_const.filter_hi_q[j];
  }
  corr = corr >> COS_BITS;
  sum += corr * corr;

  //    corr = dsp_dot_prod(fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size,
  //			fsk_const.filter_lo_i, fsk_const.filter_size, 0);
  corr = 0;
  for( j = 0; j < fsk_const.filter_size; j++ ) {
    corr += b[j] * fsk_const.filter_lo_i[j];
  }
  corr = corr >> COS_BITS;
  sum -= corr * corr;

  //    corr = dsp_dot_prod(fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size,
  //			fsk_const.filter_lo_q, fsk_const.filter_size, 0);
  corr = 0;
  for( j = 0; j < fsk_const.filter_size; j++ ) {
    corr += b[j] * fsk_const.filter_lo_q[j];
  }
  corr = corr >> COS_BITS;
  sum -= corr * corr;

  return sum;
}
#endif

void FSKdemod(demod_sample_t *samples, uint32_t nb, put_bit_func put_bit)
{
  int32_t newsample;
  uint32_t i;
  int32_t sum;
  demod_sample_t *b;
  //  int16_t tempo;

  //    GPIOB->PSOR |= (1 << 6);   // red
  // measure time to complete: 133.2us
  // number of samples processed: 8 -> needs to get to 106us, ideally 100us for OS overhead

  for (i = 0; i < nb; i++) {
    /* add a new sample in the demodulation filter */
    fsk_state.filter_buf[fsk_state.buf_ptr++] = samples[i] >> fsk_state.shift;
    if (fsk_state.buf_ptr == FSK_FILTER_BUF_SIZE) {
      memmove_aligned(fsk_state.filter_buf,
	      fsk_state.filter_buf + FSK_FILTER_BUF_SIZE - fsk_const.filter_size,
	      fsk_const.filter_size * sizeof(demod_sample_t));
      fsk_state.buf_ptr = fsk_const.filter_size;
    }

    b = fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size;

    sum = FSK_core(b);

    //tempo = (s16) (sum / 65536.0);
    //fwrite(&tempo, 1, sizeof(s16), fout);

    newsample = sum > 0;
#if 0     // report demodulator discriminator status to an IO pin to monitor signal quality
    palSetPadMode(IOPORT1, 12, PAL_MODE_OUTPUT_PUSHPULL);
    if( newsample )
      GPIOA->PSOR |= (1 << 12);   // sets to high
    else
      GPIOA->PCOR |= (1 << 12);   // clears to low
#endif

    /* baud PLL synchronisation : when we see a transition of
       frequency, we tend to modify the baud phase so that it is
       in the middle of two bits */
    if (fsk_state.lastsample != newsample) {
      fsk_state.lastsample = newsample;
      //            printf("pll=%0.3f (%d)\n", fsk_state.baud_pll / 65536.0, newsample);
      if (fsk_state.baud_pll < 0x8000)
        fsk_state.baud_pll += fsk_state.baud_pll_adj;
      else
        fsk_state.baud_pll -= fsk_state.baud_pll_adj;
    }

    fsk_state.baud_pll += fsk_state.baud_incr;

    if (fsk_state.baud_pll >= 0x10000) {
      fsk_state.baud_pll -= 0x10000;
      //            printf("baud=%f (%d)\n", fsk_state.baud_pll / 65536.0, fsk_state.lastsample);
      put_bit(fsk_state.lastsample);
    }
  }

  //    GPIOB->PCOR |= (1 << 6);   // red
}

void demodInit(void) {
  int32_t a;

  fsk_state.baud_incr = ((int32_t) fsk_const.baud_rate * 0x10000L) / (int32_t) fsk_const.sample_rate;
  fsk_state.baud_pll = 0;
  fsk_state.baud_pll_adj = fsk_state.baud_incr / 4;

  memset_aligned(fsk_state.filter_buf, 0, sizeof(fsk_state.filter_buf));
  fsk_state.buf_ptr = fsk_const.filter_size;
  fsk_state.lastsample = 0;

  fsk_state.shift = -2;
  a = fsk_const.filter_size;
  while (a != 0) {
    fsk_state.shift++;
    a /= 2;
  }
}
