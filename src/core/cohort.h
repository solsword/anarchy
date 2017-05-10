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
static inline id myc_cohort(id outer, id cohort_size, id seed) {
  return ((outer + seed) / cohort_size);
}

// Identifies our within-cohort id.
static inline id myc_cohort_inner(id outer, id cohort_size, id seed) {
  return (outer + seed) % cohort_size;
}

// Combines cohort and cohort_inner, returning both values via return
// parameters.
static inline void myc_cohort_and_inner(
  id outer,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  *r_cohort = myc_cohort(outer, cohort_size, seed);
  *r_inner = myc_cohort_inner(outer, cohort_size, seed);
}

// Find global ID from cohort & inner ID:
static inline id myc_cohort_outer(
  id cohort,
  id inner,
  id cohort_size,
  id seed
) {
  id cohort_start = cohort * cohort_size;
  return cohort_start + inner - seed;
}

// Interleaves cohort members by folding top half into bottom half.
static inline id myc_cohort_interleave(id inner, id cohort_size) {
  id bottom = (inner < ((cohort_size+1)/2));
  return (
     bottom * (inner*2) // if
  + !bottom * ((cohort_size - 1 - inner) * 2 + 1) // else
  );
}

// Reverse
static inline id myc_rev_cohort_interleave(id shuffled, id cohort_size) {
  id odd = (shuffled % 2);
  return (
     odd * (cohort_size - 1 - shuffled/2) // if
  + !odd * (shuffled/2) // else
  );
}

// Folds items past an arbitrary split point into the middle of the cohort.
// The split will always leave an odd number at the end
static inline id myc_cohort_fold(id inner, id cohort_size, id seed) {
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

static inline id myc_rev_cohort_fold(id folded, id cohort_size, id seed) {
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
static inline id myc_cohort_spin(id inner, id cohort_size, id seed) {
  return (inner + seed) % cohort_size;
}

// Reverse
static inline id myc_rev_cohort_spin(id spun, id cohort_size, id seed) {
  return (spun + (cohort_size - (seed % cohort_size))) % cohort_size;
}

// Flops cohort sections with their neighbors.
static inline id myc_cohort_flop(id inner, id cohort_size, id seed) {
  id limit = cohort_size >> 3;
  limit += (limit < 4) * 4;
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
static inline id myc_cohort_shuffle(id inner, id cohort_size, id seed) {
  id r = inner;
  r = myc_cohort_interleave(r, cohort_size);
  r = myc_cohort_spin(r, cohort_size, seed + 1982);
  r = myc_cohort_fold(r, cohort_size, seed + 837);
  r = myc_cohort_interleave(r, cohort_size);
  r = myc_cohort_flop(r, cohort_size, seed + 53);
  r = myc_cohort_fold(r, cohort_size, seed + 201);
  r = myc_cohort_interleave(r, cohort_size);
  r = myc_cohort_flop(r, cohort_size, seed + 192);
  r = myc_cohort_spin(r, cohort_size, seed + 19);
  return r;
}

// Reverse
static inline id myc_rev_cohort_shuffle(id shuffled, id cohort_size, id seed) {
  id r = shuffled;
  r = myc_rev_cohort_spin(r, cohort_size, seed + 19);
  r = myc_cohort_flop(r, cohort_size, seed + 192);
  r = myc_rev_cohort_interleave(r, cohort_size);
  r = myc_rev_cohort_fold(r, cohort_size, seed + 201);
  r = myc_cohort_flop(r, cohort_size, seed + 53);
  r = myc_rev_cohort_interleave(r, cohort_size);
  r = myc_rev_cohort_fold(r, cohort_size, seed + 837);
  r = myc_rev_cohort_spin(r, cohort_size, seed + 1982);
  r = myc_rev_cohort_interleave(r, cohort_size);
  return r;
}

// A cohort of the given size drawn from a double-wide segment of the outer
// region with 50% representation. Note that the inner indices of mixed cohorts
// are shuffled, but the bottom 1/2 indices always come earlier than the top
// 1/2 indices. Simply re-shuffle the inner results if you wish to have mixed
// indices (the biased indices are useful for some purposes).
// TODO: Guarantee that size isn't off-by-one?
static inline id myc_mixed_cohort(id outer, id cohort_size, id seed) {
  id strict_cohort = myc_cohort(outer, cohort_size, seed);
  id strict_inner = myc_cohort_inner(outer, cohort_size, seed);

  id shuf = myc_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  return (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );
}

// Gets the inner id for a mixed cohort (see above)
static inline id myc_mixed_cohort_inner(id outer, id cohort_size, id seed) {
  id strict_cohort = myc_cohort(outer, cohort_size, seed);
  id strict_inner = myc_cohort_inner(outer, cohort_size, seed);

  id shuf = myc_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);

  return shuf;
}

// Combines mixed_cohort and mixed_cohort_inner, returning both values via
// return parameters. More efficient if you need both values.
static inline void myc_mixed_cohort_and_inner(
  id outer,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id strict_cohort = myc_cohort(outer, cohort_size, seed);
  id strict_inner = myc_cohort_inner(outer, cohort_size, seed);

  id shuf = myc_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  *r_cohort = (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );

  *r_inner = shuf;
}

// Reverse
static inline id myc_mixed_cohort_outer(
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

  id unshuf = myc_rev_cohort_shuffle(inner, cohort_size, seed + strict_cohort);
  return myc_cohort_outer(strict_cohort, unshuf, cohort_size, seed);
}

// A special kind of mixed cohort that's biased towards one direction on the
// base continuum. The bias value must be between 1 and MAX_BIAS, where a value
// of MID_BIAS combines evenly. Returns via the two return parameters r_cohort
// and r_inner.
#define MAX_BIAS 32
#define MID_BIAS 16
static inline void myc_biased_cohort_and_inner(
  id outer,
  id bias,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  assert(bias > 0);
  assert(bias < MAX_BIAS);

  id strict_cohort = myc_cohort(outer, cohort_size, seed);
  id strict_inner = myc_cohort_inner(outer, cohort_size, seed);

  id shuf = myc_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id split = (cohort_size * (MAX_BIAS - bias)) / MAX_BIAS;
  id lower = shuf < split;

  *r_cohort = (
     lower * (strict_cohort + 1)
  + !lower * strict_cohort
  );
  *r_inner = shuf;
}

// Reverse
static inline id myc_biased_cohort_outer(
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

  id unshuf = myc_rev_cohort_shuffle(inner, cohort_size, seed + strict_cohort);
  return myc_cohort_outer(strict_cohort, unshuf, cohort_size, seed);
}

// Takes a float (should be between 0 and 1) and returns the nearest
// corresponding bias value, according to the resolution established by
// the definition of MAX_BIAS.
static inline id myc_nearest_bias(float f) {
  id result = (id) roundf(MAX_BIAS * f);
  if (result < 1) {
    return 1;
  } else if (result >= MAX_BIAS) {
    return MAX_BIAS - 1;
  }
  return result;
}

// Computes the cutoff for cohort flopping for exponential cohorts. Arguments
// are:
// 
//   shape controls how steeply exponential the distribution is. Values between
//   1 (essentially no cutoffs) and 0.00000000001 (almost all cutoff except in
//   the first few sections) make sense, with a few sensible ones being:
//
//     0.00001: gives ~75% full cutoff
//     0.001: gives ~50% full cutoff
//     0.01: gives ~25% full cutoff
//     0.05: leaves ~1 item in the last bin
//     0.25: leaves 25% in the last bin (only slightly curvy)
//     0.5: leaves 50% in the last bin (starts seeming linear)
//     0.75: leaves 75% in the last bin (pretty much linear)
//
//   Negative values for shape simply reverse the section position within the
//   number of sections, so that the result is symmetric to positive values.
//
//   sections indicates how many sections there are in total.
//
//   which indicates which section's cutoff we're interested in.
//
//   section_width indicates how wide each section is.
//
static inline id myc_exp_split(
  float shape,
  id sections,
  id which,
  id section_width
) {
  if (shape < 0) {
    which = sections - which - 1;
    shape = -shape;
  }
  return (id) (section_width * exp(-(which/(float)sections)*-log(shape)));
}

// Divides a cohort into sections and then sends proportionally more items from
// each section to the next cohort, forming cohorts with round asymptotic
// bottoms and long tail tops (or vice versa if shape < 0; see exp_split above).
#define EXP_SECTION_RESOLUTION 32
#define MIN_SECTION_COUNT 8
#define MIN_SECTION_RESOLUTION 4
static inline void myc_exp_cohort_and_inner(
  id outer,
  float shape,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id resolution = EXP_SECTION_RESOLUTION;
  id section_count = cohort_size / resolution;
  if (section_count < MIN_SECTION_COUNT) {
    resolution = cohort_size / MIN_SECTION_COUNT;
    if (resolution < MIN_SECTION_RESOLUTION) {
      resolution = MIN_SECTION_RESOLUTION;
    }
    section_count = cohort_size / resolution;
  }

  id strict_cohort = myc_cohort(outer, cohort_size, seed);
  id strict_inner = myc_cohort_inner(outer, cohort_size, seed);

  id section = strict_inner / resolution;
  id in_section = strict_inner % resolution;
  // TODO: ID coherency between cohorts!
  id shuf = myc_cohort_shuffle(
    in_section,
    resolution,
    seed + section
  );
  id split = myc_exp_split(shape, section_count, section, resolution);
  id lower = shuf < split;

  int adjust = !lower * (-1 + 2*(shape > 0));

  *r_cohort = strict_cohort + adjust;
  *r_inner = shuf + (section * resolution);
}

static inline id myc_exp_cohort_outer(
  id cohort,
  id inner,
  float shape,
  id cohort_size,
  id seed
) {
  id resolution = EXP_SECTION_RESOLUTION;
  id section_count = cohort_size / resolution;
  if (section_count < MIN_SECTION_COUNT) {
    resolution = cohort_size / MIN_SECTION_COUNT;
    if (resolution < MIN_SECTION_RESOLUTION) {
      resolution = MIN_SECTION_RESOLUTION;
    }
    section_count = cohort_size / resolution;
  }

  id in_section = inner % resolution;
  id section = inner / resolution;

  id split = myc_exp_split(shape, section_count, section, resolution);
  id lower = in_section < split;

  int adjust = !lower * (-1 + 2*(shape > 0));

  id strict_cohort = cohort - adjust;

  id unshuf = myc_rev_cohort_shuffle(in_section, resolution, seed + section);

  id strict_inner = (section * resolution) + unshuf;

  return myc_cohort_outer(strict_cohort, strict_inner, cohort_size, seed);
}

#endif // INCLUDE_COHORT_H
