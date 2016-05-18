/* code to be run on the local computer to generate C files
   that contain constant tables for inclusion into the demodulator core */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

typedef unsigned char           byte;
typedef unsigned short          word;
typedef unsigned int            dword;

#include "esplanade_demod.h"

FSK_demod_const  fsk;

void gen_headers(FILE *of) {
  fprintf(of, "////// THIS FILE IS AUTOMATICALLY GENENERATED DO NOT EDIT\n");
  fprintf(of, "////// 'make dsptables' to build this file\n\n");
}

void gen_filter_tab(FILE *of) {
    float phase;
    int i, a;

    fsk.f_lo = F_LO;
    fsk.f_hi = F_HI;
    fsk.baud_rate = BAUD_RATE;

    fsk.sample_rate = SAMPLE_RATE;
    
    fsk.filter_size = fsk.sample_rate / fsk.baud_rate;

    /* compute the filters */
    for(i=0;i<fsk.filter_size;i++) {
        phase = 2 * M_PI * fsk.f_lo * i / (float)fsk.sample_rate;
        fsk.filter_lo_i[i] = (int) (cos(phase) * COS_BASE);
        fsk.filter_lo_q[i] = (int) (sin(phase) * COS_BASE);

        phase = 2 * M_PI * fsk.f_hi * i / (float)fsk.sample_rate;
        fsk.filter_hi_i[i] = (int) (cos(phase) * COS_BASE);
        fsk.filter_hi_q[i] = (int) (sin(phase) * COS_BASE);
    }

    fprintf(of, "  const FSK_demod_const fsk_const = {\n");
    fprintf(of, "    %6d, %6d,  // f_lo, f_hi\n", fsk.f_lo, fsk.f_hi);
    fprintf(of, "    %6d,      // sample_rate\n", fsk.sample_rate);
    fprintf(of, "    %6d,      // baud_rate\n", fsk.baud_rate);
    fprintf(of, "    %6d,      // filter_size\n", fsk.filter_size);

    fprintf(of, "    // filer_lo_i");
    for (i = 0; i < FSK_FILTER_SIZE; i++) {
      if ((i % 16) == 0)
        fprintf(of, "\n    {");
      fprintf(of, "%d, ", fsk.filter_lo_i[i]);
    }
    fprintf(of, "},\n    ");

    fprintf(of, "    // filer_lo_q");
    for (i = 0; i < FSK_FILTER_SIZE; i++) {
      if ((i % 16) == 0)
        fprintf(of, "\n    {");
      fprintf(of, "%d, ", fsk.filter_lo_q[i]);
    }
    fprintf(of, "},\n    ");

    fprintf(of, "    // filer_hi_i");
    for (i = 0; i < FSK_FILTER_SIZE; i++) {
      if ((i % 16) == 0)
        fprintf(of, "\n    {");
      fprintf(of, "%d, ", fsk.filter_hi_i[i]);
    }
    fprintf(of, "},\n    ");
    
    fprintf(of, "    // filer_hi_q");
    for (i = 0; i < FSK_FILTER_SIZE; i++) {
      if ((i % 16) == 0)
        fprintf(of, "\n    {");
      fprintf(of, "%d, ", fsk.filter_hi_q[i]);
    }
    fprintf(of, "}\n    ");
    
    fprintf(of, "  };\n");
}

void gen_footers(FILE *of) {

}

int main(int argc, char **argv) {
  FILE *of;

  of = fopen("dsptables.h", "w");
  if (of == NULL) {
    perror("dsptables.h");
    return -1;
  }
  
  gen_headers(of);
  gen_filter_tab(of);
  gen_footers(of);
  
  return 0;
}
