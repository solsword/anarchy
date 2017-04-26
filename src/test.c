#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

int main(int argc, char** argv) {
  int i;

  id x = 10290192;

  fprintf(stdout, "Testing 'fold':\n");
  fprintf(stdout, "Start: %lu\n", x);
  for (i = 0; i < 4; ++i) {
    x = fold(x, 17 + i);
    fprintf(stdout, "Forward #%d: %lu\n", i, x);
  }

  for (i = 0; i < 4; ++i) {
    x = fold(x, 17 + (3 - i));
    fprintf(stdout, "Backwards #%d: %lu\n", i, x);
  }

  fprintf(stdout, "Testing 'circular_shift':\n");
  fprintf(stdout, "Start: %lu\n", x);
  for (i = 0; i < 4; ++i) {
    x = circular_shift(x, 17 + i);
    fprintf(stdout, "Forward #%d: %lu\n", i, x);
  }

  for (i = 0; i < 4; ++i) {
    x = rev_circular_shift(x, 17 + (3 - i));
    fprintf(stdout, "Backwards #%d: %lu\n", i, x);
  }

  fprintf(stdout, "Testing 'flop':\n");
  fprintf(stdout, "Start: %lu\n", x);
  for (i = 0; i < 2; ++i) {
    x = flop(x);
    fprintf(stdout, "Forward #%d: %lu\n", i, x);
  }

  for (i = 0; i < 2; ++i) {
    x = flop(x);
    fprintf(stdout, "Backwards #%d: %lu\n", i, x);
  }

  fprintf(stdout, "Testing 'prng':\n");
  fprintf(stdout, "Start: %lu\n", x);
  for (i = 0; i < 8; ++i) {
    x = prng(x, 17 + i);
    fprintf(stdout, "Forward #%d: %lu\n", i, x);
  }

  for (i = 0; i < 8; ++i) {
    x = rev_prng(x, 17 + (7 - i));
    fprintf(stdout, "Backwards #%d: %lu\n", i, x);
  }

  x = 3;
  fprintf(stdout, "Testing 'prng':\n");
  fprintf(stdout, "Start: %lu\n", x);
  for (i = 0; i < 8; ++i) {
    x = prng(x, 17 + i);
    fprintf(stdout, "Forward #%d: %lu\n", i, x);
  }

  for (i = 0; i < 8; ++i) {
    x = rev_prng(x, 17 + (7 - i));
    fprintf(stdout, "Backwards #%d: %lu\n", i, x);
  }

  x = 0;
  fprintf(stdout, "Testing 'prng':\n");
  fprintf(stdout, "Start: %lu\n", x);
  for (i = 0; i < 8; ++i) {
    x = prng(x, 17 + i);
    fprintf(stdout, "Forward #%d: %lu\n", i, x);
  }

  for (i = 0; i < 8; ++i) {
    x = rev_prng(x, 17 + (7 - i));
    fprintf(stdout, "Backwards #%d: %lu\n", i, x);
  }

  id y = 65;
  fprintf(stdout, "Spew test:\n");
  for (int line = 0; line < 30; ++line) {
    for (i = 0; i < 80; ++i) {
      y = prng(y, 45);
      fputc((char) (65 + (y % 26)), stdout);
    }
    fputc('\n', stdout);
  }

  fprintf(stdout, "Spew test:\n");
  for (int line = 0; line < 30; ++line) {
    for (i = 0; i < 80; ++i) {
      y = prng(y, 45);
      fputc((char) (32 + (y % 10)), stdout);
    }
    fputc('\n', stdout);
  }

  return EXIT_SUCCESS;
}
