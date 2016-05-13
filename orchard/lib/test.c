#include <stdint.h>
#include <stdio.h>

double x = 4.2;

int main(int argc, char **argv) {
  x += argc;
  fprintf(stderr, "%lf", x);
  return 0;
}
