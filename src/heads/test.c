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
#include <sys/stat.h> // for permissions

#include "tests/unit_tests.cf"
#include "tests/cohort_tests.cf"
#include "tests/select_tests.cf"
#include "tests/family_tests.cf"

void acy_unit_test(char const * const name, int (*test)(void)) {
  // TODO: Record failures in a summary.
  int result = test();
  if (result) {
    fprintf(stdout, "%40s ... failed [%d]\n", name, result);
  } else {
    fprintf(stdout, "%40s ... passed\n", name);
  }
}

int main(int argc, char** argv) {

  fprintf(stdout, "Creating test directories...\n");

  mkdir(
    "test",
    S_IRUSR | S_IWUSR | S_IXUSR
  | S_IRGRP | S_IXGRP
  | S_IROTH | S_IXOTH
  );
  // TODO: Check errno for non-EEXIST value?

  fprintf(stdout, "Testing...\n");

  #include "tests/do_unit_tests.cf"

  #include "tests/do_cohort_tests.cf"

  #include "tests/do_select_tests.cf"

  #include "tests/do_family_tests.cf"

  fprintf(stdout, "... all tests completed.\n");

  return EXIT_SUCCESS;
}
