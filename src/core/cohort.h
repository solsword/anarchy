/**
 * @file: cohort.h
 *
 * @description: Reversible operations on cohorts.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#ifndef INCLUDE_COHORT_H
#define INCLUDE_COHORT_H

#include <assert.h>
#include <math.h> // for roundf

#include "core/unit.h" // for "id" and unit operations

/********************
 * Inline Functions *
 ********************/

// Picks out which cohort we're in.
static inline id cohort(id outer, id cohort_size, id seed) {
  return ((outer + seed) / cohort_size);
}

// Identifies our within-cohort id.
static inline id cohort_inner(id outer, id cohort_size, id seed) {
  return (outer + seed) % cohort_size;
}

// Combines cohort and cohort_inner, returning both values via return
// parameters.
static inline void cohort_and_inner(
  id outer,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  *r_cohort = cohort(outer, cohort_size, seed);
  *r_inner = cohort_inner(outer, cohort_size, seed);
}

// Find global ID from cohort & inner ID:
static inline id cohort_outer(
  id cohort,
  id inner,
  id cohort_size,
  id seed
) {
  id cohort_start = cohort * cohort_size;
  return cohort_start + inner - seed;
}

// Interleaves cohort members by folding top half into bottom half.
static inline id cohort_interleave(id inner, id cohort_size) {
  id bottom = (inner < (cohort_size/2));
  return (
     bottom * (inner*2) // if
  + !bottom * ((cohort_size - 1 - inner) * 2 + 1) // else
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
  id out = result >= cohort_size;
  return (
     out * inner
  + !out * result
  );
}
// Flop is its own inverse.

// Uses the above functions to shuffle a cohort
static inline id cohort_shuffle(id inner, id cohort_size, id seed) {
  id r = inner;
  r = cohort_interleave(r, cohort_size);
  r = cohort_spin(r, cohort_size, seed + 1982);
  r = cohort_fold(r, cohort_size, seed + 837);
  r = cohort_interleave(r, cohort_size);
  r = cohort_flop(r, cohort_size, seed + 53);
  r = cohort_fold(r, cohort_size, seed + 201);
  r = cohort_interleave(r, cohort_size);
  r = cohort_flop(r, cohort_size, seed + 192);
  r = cohort_spin(r, cohort_size, seed + 19);
  return r;
}

// Reverse
static inline id rev_cohort_shuffle(id shuffled, id cohort_size, id seed) {
  id r = shuffled;
  r = rev_cohort_spin(r, cohort_size, seed + 19);
  r = cohort_flop(r, cohort_size, seed + 192);
  r = rev_cohort_interleave(r, cohort_size);
  r = rev_cohort_fold(r, cohort_size, seed + 201);
  r = cohort_flop(r, cohort_size, seed + 53);
  r = rev_cohort_interleave(r, cohort_size);
  r = rev_cohort_fold(r, cohort_size, seed + 837);
  r = rev_cohort_spin(r, cohort_size, seed + 1982);
  r = rev_cohort_interleave(r, cohort_size);
  return r;
}

// A cohort of the given size drawn from a double-wide segment of the outer
// region with 50% representation. Note that the inner indices of mixed cohorts
// are shuffled, but the bottom 1/2 indices always come earlier than the top
// 1/2 indices. Simply re-shuffle the inner results if you wish to have mixed
// indices (the biased indices are useful for some purposes).
// TODO: Guarantee that size isn't off-by-one?
static inline id mixed_cohort(id outer, id cohort_size, id seed) {
  id strict_cohort = cohort(outer, cohort_size, seed);
  id strict_inner = cohort_inner(outer, cohort_size, seed);

  id shuf = cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  return (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );
}

// Gets the inner id for a mixed cohort (see above)
static inline id mixed_cohort_inner(id outer, id cohort_size, id seed) {
  id strict_cohort = cohort(outer, cohort_size, seed);
  id strict_inner = cohort_inner(outer, cohort_size, seed);

  id shuf = cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);

  return shuf;
}

// Combines mixed_cohort and mixed_cohort_inner, returning both values via
// return parameters. More efficient if you need both values.
static inline void mixed_cohort_and_inner(
  id outer,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id strict_cohort = cohort(outer, cohort_size, seed);
  id strict_inner = cohort_inner(outer, cohort_size, seed);

  id shuf = cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  *r_cohort = (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );

  *r_inner = shuf;
}

// Reverse
static inline id mixed_cohort_outer(
  id cohort,
  id inner,
  id cohort_size,
  id seed
) {
  id lower = inner < cohort_size/2;
  id strict_cohort = (
     lower * (cohort - 1)
  + !lower * cohort
  );

  id unshuf = rev_cohort_shuffle(inner, cohort_size, seed + strict_cohort);
  return cohort_outer(strict_cohort, unshuf, cohort_size, seed);
}

// A special kind of mixed cohort that's biased towards one direction on the
// base continuum. The bias value must be between 1 and MAX_BIAS, where a value
// of MID_BIAS combines evenly. Returns via the two return parameters r_cohort
// and r_inner.
#define MAX_BIAS 32
#define MID_BIAS 16
static inline void biased_cohort_and_inner(
  id outer,
  id bias,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  assert(bias > 0);
  assert(bias < MAX_BIAS);

  id strict_cohort = cohort(outer, cohort_size, seed);
  id strict_inner = cohort_inner(outer, cohort_size, seed);

  id shuf = cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id split = (cohort_size * (MAX_BIAS - bias)) / MAX_BIAS;
  id lower = shuf < split;

  *r_cohort = (
     lower * (strict_cohort + 1)
  + !lower * strict_cohort
  );
  *r_inner = shuf;
}

// Reverse
static inline id biased_cohort_outer(
  id cohort,
  id inner,
  id bias,
  id cohort_size,
  id seed
) {
  assert(bias > 0);
  assert(bias < MAX_BIAS);

  id split = (cohort_size * (MAX_BIAS - bias)) / MAX_BIAS;

  id lower = inner < split;

  id strict_cohort = (
     lower * (cohort - 1)
  + !lower * cohort
  );

  id unshuf = rev_cohort_shuffle(inner, cohort_size, seed + strict_cohort);
  return cohort_outer(strict_cohort, unshuf, cohort_size, seed);
}

// Takes a float (should be between 0 and 1) and returns the nearest
// corresponding bias value, according to the resolution established by
// the definition of MAX_BIAS.
static inline id nearest_bias(float f) {
  id result = (id) roundf(MAX_BIAS * f);
  if (result < 1) {
    return 1;
  } else if (result >= MAX_BIAS) {
    return MAX_BIAS - 1;
  }
  return result;
}

#endif // INCLUDE_COHORT_H
