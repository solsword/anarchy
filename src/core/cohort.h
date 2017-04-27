/**
 * @file: cohort.h
 *
 * @description: Reversible operations on cohorts.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#ifndef INCLUDE_COHORT_H
#define INCLUDE_COHORT_H

#include "core/unit.h" // for "id" and unit operations

/********************
 * Inline Functions *
 ********************/

// Picks out which cohort we're in.
static inline id cohort(id continuous, id cohort_size, id cohort_seed) {
  return (continuous / cohort_size) ^ cohort_seed;
}

// Identifies our within-cohort id.
static inline id cohort_inner(id continuous, id cohort_size, id cohort_seed) {
  return continuous % cohort_size;
}

// Find global ID from cohort & inner ID:
static inline id cohort_outer(
  id cohort,
  id inner,
  id cohort_size,
  id cohort_seed
) {
  id cohort_start = (cohort ^ cohort_seed) * cohort_size;
  return cohort_start + inner;
}

// Interleaves cohort members by folding top half into bottom half.
static inline id cohort_interleave(id inner, id cohort_size) {
  id bottom = (inner < (cohort_size/2));
  return (
     bottom * (inner*2) // if
  + !bottom * ((cohort_size - inner - 1) * 2 + 1) // else
  );
}

// Reverse
static inline id rev_cohort_interleave(id shuffled, id cohort_size) {
  id odd = (shuffled % 2);
  return (
     odd * (cohort_size - 1 - shuffled/2) // if
  + !odd * (shuffled/2) // else
  );
}

// Folds items past an arbitrary split point into the middle of the cohort.
// The split will always leave an odd number at the end
static inline id cohort_fold(id inner, id cohort_size, id seed) {
  id half = cohort_size >> 1;
  id split = (seed % half) + half;
  id after = (cohort_size - split);
  split += (after + 1) % 2; // force an odd split point
  after = (cohort_size - split);

  id first = inner < half - after/2;
  id third = inner >= split;
  id second = !first && !third;

  return (
    first * (inner)
  + second * (inner + after) // total shift is after indices
  + third * ((half - after/2) + (inner - split))
  );
}

static inline id rev_cohort_fold(id folded, id cohort_size, id seed) {
  id half = cohort_size >> 1;
  id split = (seed % half) + half;
  id after = (cohort_size - split);
  split += (after + 1) % 2; // force an odd split point
  after = (cohort_size - split);

  id first = folded < half - after/2;
  id third = folded > half + after/2;
  id second = !first && !third;

  return (
    first * (folded)
  + second * (split + (folded - (half - after/2)))
  + third * (folded - after)
  );
}

// Offsets cohort members in a circular manner.
static inline id cohort_spin(id inner, id cohort_size, id seed) {
  return (inner + seed) % cohort_size;
}

// Reverse
static inline id rev_cohort_spin(id spun, id cohort_size, id seed) {
  return (spun + (cohort_size - (seed % cohort_size))) % cohort_size;
}

// Flops cohort sections with their neighbors.
static inline id cohort_flop(id inner, id cohort_size, id seed) {
  id limit = cohort_size >> 3;
  id size = (seed % limit) + 2;

  id which = inner / size;
  id local = inner % size;

  id odd = which % 2;

  id result = (
    !odd * ((which + 1)*size + local)
  +  odd * ((which - 1)*size + local)
  );
  id out = result > cohort_size;
  return (
     out * inner
  + !out * result
  );
}
// Flop is its own inverse.

// Uses the above functions to shuffle a cohort
static inline id cohort_shuffle(id inner, id cohort_size, id seed) {
  id r = inner;
//  r = cohort_flop(r, cohort_size, seed + 53);
  r = cohort_interleave(r, cohort_size);
//  r = cohort_spin(r, cohort_size, seed + 1982);
  r = cohort_fold(r, cohort_size, seed + 837);
  r = cohort_interleave(r, cohort_size);
  r = cohort_flop(r, cohort_size, seed + 192);
//  r = cohort_spin(r, cohort_size, seed + 19);
  return r;
}

// Reverse
static inline id rev_cohort_shuffle(id shuffled, id cohort_size, id seed) {
  id r = shuffled;
//  r = rev_cohort_spin(r, cohort_size, seed + 19);
  r = cohort_flop(r, cohort_size, seed + 192);
  r = rev_cohort_interleave(r, cohort_size);
  r = rev_cohort_fold(r, cohort_size, seed + 837);
//  r = rev_cohort_spin(r, cohort_size, seed + 1982);
  r = rev_cohort_interleave(r, cohort_size);
//  r = cohort_flop(r, cohort_size, seed + 53);
  return r;
}

#endif // INCLUDE_COHORT_H
