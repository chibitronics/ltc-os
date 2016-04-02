#include <stdio.h>

void main() {
  short d;
  int data;
  FILE *fp;

  fp = fopen("test.raw", "wb");
  while( !feof(stdin) ) {
    scanf("%d", &data);
    d = (short) ((data - 2048) * 32);
    fwrite(&d, 1, sizeof(short), fp);
  }

  fclose(fp);
}
