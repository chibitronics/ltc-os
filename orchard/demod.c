#pragma GCC optimize ("O3")

#include <string.h>
#include "nil.h"
//#include "ch.h"

#include "hal.h"
#include "pal.h"

#include "analog.h"
#include "demod.h"
#include "dsptables.h"

int16_t dm_buf[DMBUF_DEPTH];

static FSK_demod_state fsk_state;

int32_t FSK_core(int16_t *b) {
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

void FSKdemod(int16_t *samples, uint32_t nb, put_bit_func put_bit)
{
  int32_t newsample;
  uint32_t i;
  int32_t sum;
  int16_t *b;
  //  int16_t tempo;

  //    GPIOB->PSOR |= (1 << 6);   // red
  // measure time to complete: 133.2us
  // number of samples processed: 8 -> needs to get to 106us, ideally 100us for OS overhead
  
  for(i = 0; i < nb; i++) {
    /* add a new sample in the demodulation filter */
    fsk_state.filter_buf[fsk_state.buf_ptr++] = samples[i] >> fsk_state.shift;
    if (fsk_state.buf_ptr == FSK_FILTER_BUF_SIZE) {
      memmove(fsk_state.filter_buf, 
	      fsk_state.filter_buf + FSK_FILTER_BUF_SIZE - fsk_const.filter_size, 
	      fsk_const.filter_size * sizeof(int16_t));
      fsk_state.buf_ptr = fsk_const.filter_size;
    }
        
    b = fsk_state.filter_buf + fsk_state.buf_ptr - fsk_const.filter_size;

    sum = FSK_core(b);
    
    //tempo = (s16) (sum / 65536.0);
    //fwrite(&tempo, 1, sizeof(s16), fout);
	
    newsample = sum > 0;

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

  memset(fsk_state.filter_buf, 0, sizeof(fsk_state.filter_buf));
  fsk_state.buf_ptr = fsk_const.filter_size;
  fsk_state.lastsample = 0;

  fsk_state.shift = -2;
  a = fsk_const.filter_size;
  while (a != 0) {
    fsk_state.shift++;
    a /= 2;
  }
}
