/**
 * @file: cohort.h
 *
 * @description: Reversible operations on cohorts.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#ifndef INCLUDE_COHORT_H
#define INCLUDE_COHORT_H

#include "unit.h" // for "id" and unit operations

/********************
 * Inline Functions *
 ********************/

// Picks out which cohort we're in.
static inline id cohort(id continuous, id cohort_size, id cohort_seed) {
  return (continuous / cohort_size) ^ cohort_seed;
}

// Identifies our within-cohort id.
static inline id within_cohort(id continuous, id cohort_size, id cohort_seed) {
  return continuous % cohort_size;
}

// Find global ID from cohort/cohort ID:
static inline id cohort_outer(
  id cohort,
  id within_cohort,
  id cohort_size,
  id cohort_seed
) {
  id cohort_start = (cohort ^ cohort_seed) * cohort_size;
  return cohort_start + within_cohort;
}

#endif // INCLUDE_COHORT_H
