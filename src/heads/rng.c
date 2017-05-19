/**
 * @file: rng.c
 *
 * @description: Runs just the core unit PRNG as an RNG spitting out bytes. If
 * a number is given as a command-line argument, it'll stop after 8× that many
 * bytes (it generates that many ids, and each id is 8 bytes).
 *
 * Suitable for piping into dieharder -g200, e.g.,
 * 
 *   rng 10000000000 | dieharder -g200 -a
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <stdio.h>
#include <stdlib.h>

#include "core/unit.h"

int main(int argc, char** argv) {
  size_t limit = 0;
  if (argc > 1) {
    if (sscanf(argv[1], "%zu", &limit) == EOF) {
      fprintf(
        stdout,
        "Error: couldn't parse '%s' as an output limit.\n",
        argv[1]
      );
      return EXIT_FAILURE;
    }
  }
  id x = 7817298123;
  id seed = 1092809123;
  size_t count = 0;
  while (limit == 0 || count < limit) {
    x = acy_prng(x, seed);
    char * byte = (char*) &x;
    for (size_t i = 0; i < sizeof(id); ++i) {
      fputc(*byte, stdout);
      byte += 1;
    }
    // fprintf(stdout, "%20lu\n", x);
    count += 1;
  }
  return EXIT_SUCCESS;
}
