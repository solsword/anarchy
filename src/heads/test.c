/**
 * @file: test.c
 *
 * @description: Runs unit tests from the src/tests folder. Add #includes here
 * for each .c file there.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <stdio.h>
#include <stdlib.h>

#include "tests/unit_tests.cf"
#include "tests/cohort_tests.cf"

void unit_test(char const * const name, int (*test)(void)) {
  // TODO: Record failures in a summary.
  int result = test();
  if (result) {
    fprintf(stdout, "%s ... failed [%d]\n", name, result);
  } else {
    fprintf(stdout, "%s ... passed\n", name);
  }
}

int main(int argc, char** argv) {

  fprintf(stdout, "Testing...\n");

  #include "tests/do_unit_tests.cf"

  #include "tests/do_cohort_tests.cf"

  fprintf(stdout, "... all tests completed.\n");

  return EXIT_SUCCESS;
}
