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
#ifdef DEBUG_COHORT
  #include <stdio.h>
#endif

#include "core/unit.h" // for "id" and unit operations

/********************
 * Inline Functions *
 ********************/

// Picks out which cohort we're in.
static inline id acy_cohort(id outer, id cohort_size) {
  return outer / cohort_size;
}

// Identifies our within-cohort id.
static inline id acy_cohort_inner(id outer, id cohort_size) {
  return outer % cohort_size;
}

// Combines cohort and cohort_inner, returning both values via return
// parameters.
static inline void acy_cohort_and_inner(
  id outer,
  id cohort_size,
  id *r_cohort,
  id *r_inner
) {
  *r_cohort = acy_cohort(outer, cohort_size);
  *r_inner = acy_cohort_inner(outer, cohort_size);
}

// Find global ID from cohort & inner ID:
static inline id acy_cohort_outer(
  id cohort,
  id inner,
  id cohort_size
) {
  id cohort_start = cohort * cohort_size;
  return cohort_start + inner;
}

// Interleaves cohort members by folding top half into bottom half.
static inline id acy_cohort_interleave(id inner, id cohort_size) {
  id bottom = (inner < ((cohort_size+1)/2));
  return (
     bottom * (inner*2) // if
  + !bottom * ((cohort_size - 1 - inner) * 2 + 1) // else
  );
}

// Reverse
static inline id acy_rev_cohort_interleave(id shuffled, id cohort_size) {
  id odd = (shuffled % 2);
  return (
     odd * (cohort_size - 1 - shuffled/2) // if
  + !odd * (shuffled/2) // else
  );
}

// Folds items past an arbitrary split point into the middle of the cohort.
// The split will always leave an odd number at the end
static inline id acy_cohort_fold(id inner, id cohort_size, id seed) {
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

static inline id acy_rev_cohort_fold(id folded, id cohort_size, id seed) {
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
static inline id acy_cohort_spin(id inner, id cohort_size, id seed) {
  return (inner + seed) % cohort_size;
}

// Reverse
static inline id acy_rev_cohort_spin(id spun, id cohort_size, id seed) {
  return (spun + (cohort_size - (seed % cohort_size))) % cohort_size;
}

// Flops cohort sections with their neighbors.
static inline id acy_cohort_flop(id inner, id cohort_size, id seed) {
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

// Does a different cohort spin for even and odd items.
static inline id acy_cohort_mix(id inner, id cohort_size, id seed) {
  id even = inner - inner % 2;
  id target;
  if (inner % 2) {
    target = acy_cohort_spin(
      even/2,
      (cohort_size+(1-cohort_size%2))/2,
      seed + 464185
    );
    return 2 * target + 1;
  } else {
    target = acy_cohort_spin(even/2, (cohort_size+1)/2, seed + 1048239);
    return 2 * target;
  }
}

// Reverse
static inline id acy_rev_cohort_mix(id mixed, id cohort_size, id seed) {
  id even = mixed - mixed % 2;
  id target;
  if (mixed % 2) {
    target = acy_rev_cohort_spin(
      even/2,
      (cohort_size+(1-cohort_size%2))/2,
      seed + 464185
    );
    return 2 * target + 1;
  } else {
    target = acy_rev_cohort_spin(even/2, (cohort_size+1)/2, seed + 1048239);
    return 2 * target;
  }
}

// Spreads items out between a number of different regions.
#define MIN_REGION_SIZE 2
#define MAX_REGION_COUNT 16
// TODO: Don't send stuff out-of-range
static inline id acy_cohort_spread(id inner, id cohort_size, id seed) {
  id min_regions = 2 - (cohort_size < 2 * MIN_REGION_SIZE);
  id max_regions = 1 + cohort_size / MIN_REGION_SIZE;
  id regions = (
    min_regions
  + (
      (seed % (1 + (max_regions - min_regions)))
    % MAX_REGION_COUNT
    )
  );
  id region_size = cohort_size / regions;
  id leftovers = cohort_size - regions * region_size;

  id region = inner % regions;
  id index = inner / regions;
  return (
     (index < region_size) * (region * region_size + index + leftovers)
  + !(index < region_size) * (inner - regions * region_size)
  );
}

// Reverse
static inline id acy_rev_cohort_spread(id spread, id cohort_size, id seed) {
  id min_regions = 2 - (cohort_size < 2 * MIN_REGION_SIZE);
  id max_regions = 1 + cohort_size / MIN_REGION_SIZE;
  id regions = (
    min_regions
  + (
      (seed % (1 + (max_regions - min_regions)))
    % MAX_REGION_COUNT
    )
  );
  id region_size = cohort_size / regions;
  id leftovers = cohort_size - regions * region_size;

  id index = (spread - leftovers) / region_size;
  id region = (spread - leftovers) % region_size;
  return (
     (spread < leftovers) * (regions * region_size + spread)
  + !(spread < leftovers) * (region * regions + index)
  );
}

// Reverses ordering within each of several fragments.
static inline id acy_cohort_upend(id inner, id cohort_size, id seed) {
  id min_regions = 2 - (cohort_size < 2 * MIN_REGION_SIZE);
  id max_regions = 1 + cohort_size / MIN_REGION_SIZE;
  id regions = (
    min_regions
  + (
      (seed % (1 + (max_regions - min_regions)))
    % MAX_REGION_COUNT
    )
  );
  id region_size = cohort_size / regions;

  id region = inner / region_size;
  id index = inner % region_size;
  id result = region * region_size + (region_size - 1 - index);
  return (
     (result < cohort_size) * result
  + !(result < cohort_size) * inner
  );
}
// Upend is its own inverse

// Uses the above functions to shuffle a cohort
static inline id acy_cohort_shuffle(id inner, id cohort_size, id seed) {
#ifdef DEBUG_COHORT
  if (cohort_size == 1) {
    fprintf(stderr, "Cohort shuffle with size==1!\n");
    return inner;
  }
#endif
  id r = inner;
  seed ^= cohort_size/3;
  r = acy_cohort_spread(r, cohort_size, seed + 453);
  r = acy_cohort_mix(r, cohort_size, seed + 2891);
  r = acy_cohort_interleave(r, cohort_size);
  r = acy_cohort_spin(r, cohort_size, seed + 1982);
  r = acy_cohort_upend(r, cohort_size, seed + 47);
  r = acy_cohort_fold(r, cohort_size, seed + 837);
  r = acy_cohort_interleave(r, cohort_size);
  r = acy_cohort_flop(r, cohort_size, seed + 53);
  r = acy_cohort_fold(r, cohort_size, seed + 201);
  r = acy_cohort_mix(r, cohort_size, seed + 728);
  r = acy_cohort_spread(r, cohort_size, seed + 881);
  r = acy_cohort_interleave(r, cohort_size);
  r = acy_cohort_flop(r, cohort_size, seed + 192);
  r = acy_cohort_upend(r, cohort_size, seed + 794614);
  r = acy_cohort_spin(r, cohort_size, seed + 19);
  return r;
}

// Reverse
static inline id acy_rev_cohort_shuffle(id shuffled, id cohort_size, id seed) {
#ifdef DEBUG_COHORT
  if (cohort_size == 1) {
    fprintf(stderr, "Reverse cohort shuffle with size==1!\n");
    return shuffled;
  }
#endif
  id r = shuffled;
  seed ^= cohort_size/3;
  r = acy_rev_cohort_spin(r, cohort_size, seed + 19);
  r = acy_cohort_upend(r, cohort_size, seed + 794614);
  r = acy_cohort_flop(r, cohort_size, seed + 192);
  r = acy_rev_cohort_interleave(r, cohort_size);
  r = acy_rev_cohort_spread(r, cohort_size, seed + 881);
  r = acy_rev_cohort_mix(r, cohort_size, seed + 728);
  r = acy_rev_cohort_fold(r, cohort_size, seed + 201);
  r = acy_cohort_flop(r, cohort_size, seed + 53);
  r = acy_rev_cohort_interleave(r, cohort_size);
  r = acy_rev_cohort_fold(r, cohort_size, seed + 837);
  r = acy_cohort_upend(r, cohort_size, seed + 47);
  r = acy_rev_cohort_spin(r, cohort_size, seed + 1982);
  r = acy_rev_cohort_interleave(r, cohort_size);
  r = acy_rev_cohort_mix(r, cohort_size, seed + 2891);
  r = acy_rev_cohort_spread(r, cohort_size, seed + 453);
  return r;
}

// A cohort of the given size drawn from a double-wide segment of the outer
// region with 50% representation. Note that the inner indices of mixed cohorts
// are shuffled, but the bottom 1/2 indices always come earlier than the top
// 1/2 indices. Simply re-shuffle the inner results if you wish to have mixed
// indices (the biased indices are useful for some purposes).
// TODO: Guarantee that size isn't off-by-one?
static inline id acy_mixed_cohort(id outer, id cohort_size, id seed) {
  id strict_cohort = acy_cohort(outer, cohort_size);
  id strict_inner = acy_cohort_inner(outer, cohort_size);

  id shuf = acy_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  return (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );
}

// Gets the inner id for a mixed cohort (see above)
static inline id acy_mixed_cohort_inner(id outer, id cohort_size, id seed) {
  id strict_cohort = acy_cohort(outer, cohort_size);
  id strict_inner = acy_cohort_inner(outer, cohort_size);

  id shuf = acy_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);

  return shuf;
}

// Combines acy_mixed_cohort and acy_mixed_cohort_inner, returning both values
// via return parameters. More efficient if you need both values.
static inline void acy_mixed_cohort_and_inner(
  id outer,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id strict_cohort = acy_cohort(outer, cohort_size);
  id strict_inner = acy_cohort_inner(outer, cohort_size);

  id shuf = acy_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  *r_cohort = (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );

  *r_inner = shuf;
}

// Reverse
static inline id acy_mixed_cohort_outer(
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

  id unshuf = acy_rev_cohort_shuffle(inner, cohort_size, seed + strict_cohort);
  return acy_cohort_outer(strict_cohort, unshuf, cohort_size);
}

// A special kind of mixed cohort that's biased towards one direction on the
// base continuum. The bias value must be between 1 and MAX_BIAS, where a value
// of MID_BIAS combines evenly. Returns via the two return parameters r_cohort
// and r_inner.
#define MAX_BIAS 32
#define MID_BIAS 16
static inline void acy_biased_cohort_and_inner(
  id outer,
  id bias,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  assert(bias > 0);
  assert(bias < MAX_BIAS);

  id strict_cohort = acy_cohort(outer, cohort_size);
  id strict_inner = acy_cohort_inner(outer, cohort_size);

  id shuf = acy_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id split = (cohort_size * (MAX_BIAS - bias)) / MAX_BIAS;
  id lower = shuf < split;

  *r_cohort = (
     lower * (strict_cohort + 1)
  + !lower * strict_cohort
  );
  *r_inner = shuf;
}

// Reverse
static inline id acy_biased_cohort_outer(
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

  id unshuf = acy_rev_cohort_shuffle(inner, cohort_size, seed + strict_cohort);
  return acy_cohort_outer(strict_cohort, unshuf, cohort_size);
}

// Takes a double (should be between 0 and 1) and returns the nearest
// corresponding bias value, according to the resolution established by
// the definition of MAX_BIAS.
static inline id acy_nearest_bias(double f) {
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
//   0.0...01 and infinity make sense, with smaller values being higher
//   logarithmic shapes and larger values being deeper exponential shapes.
//   A few sensible values are:
//
//     0.0001
//     0.001
//     0.01
//     0.05
//     2
//     10
//     100
//     1000
//
//   Negative values for shape simply reverse the section position within the
//   number of sections, so that the result is symmetric to positive values.
//
//   sections indicates how many sections there are in total.
//
//   section_width indicates how wide each section is.
//
//   which indicates which section's cutoff we're interested in.
//
static inline id acy_exp_split(
  double shape,
  id section_count,
  id section_width,
  id which
) {
  if (shape < 0) {
    which = section_count - which - 1;
    shape = -shape;
  }
  double x = which / (double) section_count;
  double f = (pow(shape, -x) - 1/shape) / (1 - 1/shape);
  return (id) (section_width * f);
  // TODO: Get rid of this old equation
  //return (id) (section_width * exp(-(which/(double)section_count)*-log(shape)));
}

// Computes the section count, section width, and leftovers for a cohort of the
// given size.
#define EXP_SECTION_RESOLUTION 1024
#define MIN_SECTION_COUNT 8
#define MIN_SECTION_RESOLUTION 4
static inline void acy_get_section_info(
  id cohort_size,
  id *r_section_count,
  id *r_section_width,
  id *r_leftovers
) {
  *r_section_width = EXP_SECTION_RESOLUTION;
  *r_section_count = cohort_size / *r_section_width;
  if (*r_section_count < MIN_SECTION_COUNT) {
    *r_section_width = cohort_size / MIN_SECTION_COUNT;
    if (*r_section_width < MIN_SECTION_RESOLUTION) {
      *r_section_width = MIN_SECTION_RESOLUTION;
    }
    *r_section_count = cohort_size / *r_section_width;
  }
  *r_leftovers = cohort_size - (*r_section_count * *r_section_width);
}

// Divides a cohort into sections and then sends proportionally more items from
// each section to the next cohort, forming cohorts with round asymptotic
// bottoms and long tail tops (or vice versa if shape < 1; see exp_split above).
// See also acy_multiexp_cohort_and_inner below, which gives nicer
// distributions at the expense of inner ID completeness/continuity.
static inline void acy_exp_cohort_and_inner(
  id outer,
  double shape,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id section_count, section_width, leftovers;
  acy_get_section_info(
    cohort_size,
    &section_count,
    &section_width,
    &leftovers
  );

  id strict_cohort = acy_cohort(outer, cohort_size);
  id strict_inner = acy_cohort_inner(outer, cohort_size);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\nexp_cohort::outer/shape/size::%lu/%.3f/%lu\n",
    outer,
    shape,
    cohort_size
  );
  fprintf(
    stderr,
    "exp_cohort::strict_cohort/inner::%lu/%lu\n",
    strict_cohort,
    strict_inner
  );
#endif

  id section = strict_inner / section_width;
  id in_section = strict_inner % section_width;
  // Note: ID coherency between cohorts is impossible if we also want a smooth
  // distribution of cohort members throughout ID space (which is much more
  // important)
  id shuf = acy_cohort_shuffle(
    in_section,
    section_width,
    seed + section
  );
  id split = acy_exp_split(shape, section_count, section_width, section);
  id lower = shuf < split;

  // TODO: Type here?
  int adjust = !lower * (-1 + 2*(shape > 0));

#ifdef DEBUG_COHORT
  fprintf(stderr, "exp_cohort::adjust::%d\n\n", adjust);
#endif

  *r_cohort = strict_cohort + adjust;
  *r_inner = shuf + (section * section_width);
}

static inline id acy_exp_cohort_outer(
  id cohort,
  id inner,
  double shape,
  id cohort_size,
  id seed
) {
  id section_count, section_width, leftovers;
  acy_get_section_info(
    cohort_size,
    &section_count,
    &section_width,
    &leftovers
  );

  id in_section = inner % section_width;
  id section = inner / section_width;

  id split = acy_exp_split(shape, section_count, section_width, section);
  id lower = in_section < split;

  int adjust = !lower * (-1 + 2*(shape > 0));

  id strict_cohort = cohort - adjust;

  id unshuf = acy_rev_cohort_shuffle(in_section, section_width, seed + section);

  id strict_inner = (section * section_width) + unshuf;

  return acy_cohort_outer(strict_cohort, strict_inner, cohort_size);
}

// Works like acy_exp_split but computes multiple layered splits, using the
// additional layer and n_layers argument to select a layer. Note that section
// indices (which) are relative to cohort -1, so a section index of
// section_count is the 0th section of the current cohort.
static inline id acy_multiexp_split(
  double shape,
  id section_count,
  id section_width,
  id which,
  id layer,
  id n_layers
) {
  // TODO: What if this rounds?
  id layer_width = section_count / n_layers; // in sections
  int adjust;
  if (shape > 0) {
    adjust = -layer * layer_width; // in sections
  } else {
    adjust = -2*section_count + layer * layer_width; // in sections
  }
  // Cut things off after 1 full cohort:
  if (which + adjust >= section_count) {
    return 0;
  }
  return acy_exp_split(
    shape,
    section_count,
    section_width,
    which + adjust
  );
}

// Looks up which layer we're in using acy_multiexp_split. Note that the layer
// ranges from 0 to n_layers*2+1, and the sections are indexed starting from
// the beginning of the previous cohort.
static inline id acy_multiexp_get_layer(
  id section,
  id in_section,
  double shape,
  id section_count,
  id section_width,
  id n_layers
) {
  id layer = 0;
  id last_split = 0;
  id split;
  do {
    split = acy_multiexp_split(
      shape,
      section_count,
      section_width,
      section,
      layer,
      n_layers
    );
    if (split < last_split) {
      layer += 1;
      break;
    }
    last_split = split;
    layer += 1;
  } while (in_section >= split && layer < n_layers*2 + 2);
  return layer - 1;
}

// Computes the top and bottom limits of the given layer in the given section.
// Uses full section indices, so add section_count to within-section indices
// before passing to this function.
static inline void acy_multiexp_limits(
  double shape,
  id section_count,
  id section_width,
  id which,
  id layer,
  id n_layers,
  id *r_bottom,
  id *r_top
) {
  id layer_width = section_count / n_layers; // in sections
  id layer_origin_section = layer_width * layer;
  id bottom = acy_multiexp_split(
    shape,
    section_count,
    section_width,
    which,
    layer - 1,
    n_layers
  );
  id top = acy_multiexp_split(
    shape,
    section_count,
    section_width,
    which,
    layer,
    n_layers
  );
  if (bottom > top) {
    if (which < layer_origin_section) {
      top = section_width;
    } else {
      bottom = 0;
    }
  }
  if (top > section_width) { top = section_width; }
  *r_bottom = bottom;
  *r_top = top;
}

// Computes the maximum number of items that could be assigned to a single
// cohort from a given section.
static inline id acy_multiexp_max_per_section(
  double shape,
  id section_count,
  id section_width,
  id n_layers
) {
  id lower, upper;
  // Find position/cohort of maximum width:
  id layer_width = section_count / n_layers; // in sections
  id section = section_count + layer_width - 1;
  id layer = n_layers + 1;
  // Compute limits & return:
  acy_multiexp_limits(
    shape,
    section_count,
    section_width,
    section,
    layer,
    n_layers,
    &lower,
    &upper
  );
  return upper - lower;
}

// Works like acy_exp_cohort_and_inner but instead of slicing each cohort into
// two parts, it slices each cohort into multiple parts, and distributes them
// nearby. This can give a much smoother distribution.
static inline void acy_multiexp_cohort_and_inner(
  id outer,
  double shape,
  id cohort_size,
  id n_layers,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id section_count, section_width, leftovers;
  acy_get_section_info(
    cohort_size,
    &section_count,
    &section_width,
    &leftovers
  );

  id strict_cohort;
  id strict_inner;
  acy_cohort_and_inner(outer, cohort_size, &strict_cohort, &strict_inner);

#ifdef DEBUG_COHORT
  fprintf( stderr, "\nmultiexp_cohort::outer::%lu\n", outer);
  fprintf(
    stderr,
    "multiexp_cohort::shape/size::%.3f/%lu\n",
    shape,
    cohort_size
  );
  fprintf(
    stderr,
    "multiexp_cohort::strict_cohort/inner::%lu/%lu\n",
    strict_cohort,
    strict_inner
  );
#endif

  // shuffle within super-cohort:
  // id shuf = acy_cohort_shuffle(strict_inner, cohort_size, seed);

  // section information:
  id section = strict_inner / section_width;
  id full_section = section + section_count;
  id in_section = strict_inner % section_width;

  id shuf = acy_cohort_shuffle(in_section, section_width, seed + section);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_cohort::section/in_section/shuffled::%lu/%lu/%lu\n",
    section,
    in_section,
    shuf
  );
#endif

  // find layer:
  id layer = acy_multiexp_get_layer(
    full_section,
    shuf,
    shape,
    section_count,
    section_width,
    n_layers
  );

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_cohort::sections/section_width::%lu/%lu\n",
    section_count,
    section_width
  );
  fprintf(stderr, "multiexp_cohort::layer::%lu\n", layer);
#endif

  id adjusted_cohort = strict_cohort * n_layers + layer;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_cohort::adjusted_cohort/shuffled_inner::%lu/%lu\n\n",
    adjusted_cohort,
    strict_inner
  );
#endif

  if (adjusted_cohort < strict_cohort) { // overflow
    *r_cohort = NONE;
    *r_inner = NONE;
    return;
  }
  *r_cohort = adjusted_cohort;
  *r_inner = strict_inner;
}

static inline id acy_multiexp_cohort_outer(
  id cohort,
  id inner,
  double shape,
  id cohort_size,
  id n_layers,
  id seed
) {
  id section_count, section_width, leftovers;
  acy_get_section_info(
    cohort_size,
    &section_count,
    &section_width,
    &leftovers
  );

#ifdef DEBUG_COHORT
  fprintf(stderr, "\nmultiexp_outer::cohort/inner::%lu/%lu\n", cohort, inner);
  fprintf(
    stderr,
    "multiexp_outer::shape/size::%.3f/%lu\n",
    shape,
    cohort_size
  );
#endif

  // section information:
  id section = inner / section_width;
  id full_section = section + section_count;
  id in_section = inner % section_width;

  id shuf = acy_cohort_shuffle(in_section, section_width, seed + section);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_outer::section/in_section/shuffled::%lu/%lu/%lu\n",
    section,
    in_section,
    shuf
  );
#endif

  // find layer:
  id layer = acy_multiexp_get_layer(
    full_section,
    shuf,
    shape,
    section_count,
    section_width,
    n_layers
  );

#ifdef DEBUG_COHORT
  fprintf(stderr, "multiexp_outer::layer::%lu\n", layer);
#endif

  // Back out the strict cohort and layer:
  id strict_cohort = (cohort - layer) / n_layers;

  // escape super-cohort:
  id result = acy_cohort_outer(strict_cohort, inner, cohort_size);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_outer::strict_cohort/inner/result::%lu/%lu/%lu\n\n",
    strict_cohort,
    inner,
    result
  );
#endif

  return result;
}

// Computes the sum from k=1 to n of k*shape, which folds down to:
//
//   sum = shape/2 * n * (n + 1)
//
static inline id acy_quadsum(id n, id shape) {
  return (shape * n * (n + 1)) / 2;
  // Note: Integer division by 2 here is fine (see proto/sum_formulae.py).
}

// Computes the inverse of the function above (technically, returns the
// positive root of the quadratic, but we don't care about the negative one).
// So given a sum, computes the iterations N that would result in that sum.
// Rounds the result towards zero, so for inexact sums, returns an N that would
// result in the next-lower possible sum, while adding one to the result will
// be the next-higher sum.
//
// Solution:
//   https://www.wolframalpha.com/input/?i=x+%3D+1%2F2+g+n+(n%2B1)+solve+for+n
// 
//   n = ( sqrt(g + 8x) - sqrt(g) ) / 2 sqrt(g)
//   n = ( sqrt(g * (1 + 8x/g)) - sqrt(g) ) / 2 sqrt(g)
//   n = ( sqrt(g) * sqrt(1 + 8x/g) - sqrt(g) ) / 2 sqrt(g)
//   n = sqrt(g) * ( sqrt(1 + 8x/g) - 1 ) / 2 sqrt(g)
//   n = ( sqrt(4 * (1/4 + 2x/g)) - 1 ) / 2
//   n = ( sqrt(4) * sqrt(1/4 + 2x/g) - 2 * 1/2 ) / 2
//   n = 2 * ( sqrt(1/4 + 2x/g) - 1/2 ) / 2
//   n = sqrt(1/4 + 2x/g) - 1/2
//
static inline id acy_inv_quadsum(id sum, id shape) {
  return (int) floor(sqrt(0.25 + 2 * sum / shape) - 0.5);
  // Note: integer division of sum by shape should be okay here (see
  // proto/sum_formulae.py).
}

// Take the equation spread = sum * acy_inv_quadsum(sum, shape) and  searches
// for a sum that will result in the desired spread, returning both the sum and
// the acy_inv_quadsum result for that sum (via the return parameters).
//
// The full equation isn't easily solved, but we start with an approximate
// solution of sum = (spread * shape / 2)^(2/3) and explore from there.
//
// For spread values <= 1 it just returns 1, in which case depending on the
// shape the actual spread may be larger than he desired spread, but that's
// unavoidable.
static inline void acy_inv_quadspread(
  id spread,
  id shape,
  id *r_size,
  id *r_base
) {
  id sum_approx = pow(
    (2 * spread + 1) * shape / 2.0,
    2.0/3.0
  ) / pow(2, 2.0/3.0);

  id base_approx = acy_inv_quadsum(sum_approx, shape);

  if (sum_approx * base_approx > spread) {
    // This case should be quite rare.
    while (sum_approx * base_approx > spread && base_approx > 1) {
      base_approx -= 1;
      sum_approx = acy_quadsum(base_approx, shape);
    }
  } else {
    // Hopefully the usual number of steps in this case is 1 or 2.
    while (sum_approx * base_approx < spread) {
      base_approx += 1;
      sum_approx = acy_quadsum(base_approx, shape);
    }
    base_approx -= 1;
    sum_approx = acy_quadsum(base_approx, shape);
  }

  *r_size = sum_approx;
  *r_base = base_approx;
}

// For a given cohort shape and desired cohort size, computes the nearest
// workable size for acy_multipoly_cohort_and_inner/acy_multipoly_cohort_outer
// and returns (via return parameters) the computed size and the appropriate
// base to use as an argument to get that size.
static inline void acy_multipoly_nearest_cohort_size(
  id cohort_shape,
  id desired_size,
  id *r_nearest,
  id *r_base
) {
  id lower = acy_inv_quadsum(desired_size, cohort_shape);
  id lsize = acy_quadsum(lower, cohort_shape);
  id usize = acy_quadsum(lower + 1, cohort_shape);
  id ldiff = desired_size - lsize;
  id udiff = usize - desired_size;
  if (ldiff < udiff) {
    *r_nearest = lsize;
    *r_base = lower;
  } else {
    *r_nearest = usize;
    *r_base = lower + 1;
  }
}

// Works like acy_multipoly_nearest_cohort_size but always returns the
// next-smaller size even if a larger size is closer to the desired size, so
// that the result is guaranteed to be no larger than the desired size. To get
// a size that's guaranteed to be larger than the desired size (but still as
// close as possible), just add one to the base result here.
static inline void acy_multipoly_smaller_cohort_size(
  id cohort_shape,
  id desired_size,
  id *r_sufficient,
  id *r_base
) {
  id lower = acy_inv_quadsum(desired_size, cohort_shape);
  *r_sufficient = acy_quadsum(lower, cohort_shape);
  *r_base = lower;
}

// Works like acy_multiexp_cohort_and_inner but restricts possible cohort sizes
// and uses a merely polynomial distribution to ensure inner cohort ID
// completeness & continuity. See acy_multipoly_nearest_cohort_size above for
// details on computing the cohort_size_base parameter, which indirectly
// determines the cohort size. You can use acy_quadsum to determine the actual
// cohort size yourself, but note that the members of the cohort are spread
// over a region that's potentially as large as the cohort size times the
// cohort size base, so if you want to control distribution rather than cohort
// size, use acy_inv_quadspread.
//
// TODO: Find a telescoping exponential sum!!
// TODO: Support negative shapes?
static inline void acy_multipoly_cohort_and_inner(
  id outer,
  id cohort_size_base,
  id cohort_shape,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id cohort_size = acy_quadsum(cohort_size_base, cohort_shape);
  id super_size = cohort_size * cohort_size_base;

  id super_cohort;
  id super_inner;
  acy_cohort_and_inner(outer, super_size, &super_cohort, &super_inner);

#ifdef DEBUG_COHORT
  fprintf( stderr, "\nmultipoly_cohort::outer::%lu\n", outer);
  fprintf(
    stderr,
    "multipoly_cohort::shape/base/size/super::%lu/%lu/%lu/%lu\n",
    cohort_shape,
    cohort_size_base,
    cohort_size,
    super_size
  );
  fprintf(
    stderr,
    "multipoly_cohort::super_cohort/inner::%lu/%lu\n",
    super_cohort,
    super_inner
  );
#endif

  // section information:
  id section = super_inner / cohort_size;
  id in_section = super_inner % cohort_size;

  // shuffle within each section
  id shuf = acy_cohort_shuffle(in_section, cohort_size, seed + section);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_cohort::section/in_section/shuffled::%lu/%lu/%lu\n",
    section,
    in_section,
    shuf
  );
#endif

  // Figure out which slice we fall into:
  id slice = acy_inv_quadsum(shuf, cohort_shape);
  // ...and where we are in that slice:
  id before_slice = acy_quadsum(slice, cohort_shape);
  id in_slice = shuf - before_slice;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_cohort::slice/before_slice/in_slice::%lu/%lu/%lu\n",
    slice,
    before_slice,
    in_slice
  );
#endif

  // TODO: Double-check this math
  // We know that there are cohort_size_base cohorts per super_cohort, so
  // before the previous super_cohort, there were cohort_size_base *
  // (super_cohort - 1) cohorts. Now if we're slice 0 of section 0, we're
  // actually in the very last segment of the second cohort introduced during
  // the previous super_cohort. Similarly, slice 1 of section 0 is the
  // second-to-last segment of the second cohort introduced in the previous
  // super cohort. Additionally, for each section we move over, we're also
  // shifting a cohort. So section + slice + 1 gets added to find the cohort
  // we're in.
  *r_cohort = cohort_size_base * (super_cohort - 1) + section + slice + 1;
  // TODO: Detect underflow here?

  // The sum of all things previous to us within our cohort is just the
  // difference of two quadsums: the quadsum for the full cohort minus the
  // quadsum for our segment, and notice that our slice is the same as our
  // segment. So:
  id inner = cohort_size - acy_quadsum(slice, cohort_shape) - in_slice - 1;

  // TODO: DEBUG
  *r_inner = acy_cohort_shuffle(inner, cohort_size, seed + *r_cohort);
  //*r_inner = inner;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_cohort::cohort/inner::%lu/%lu\n",
    *r_cohort,
    *r_inner
  );
#endif
}

static inline id acy_multipoly_cohort_outer(
  id cohort,
  id inner,
  id cohort_size_base,
  id cohort_shape,
  id seed
) {
  id cohort_size = acy_quadsum(cohort_size_base, cohort_shape);
  id super_size = cohort_size * cohort_size_base;

  // TODO: DEBUG
  inner = acy_rev_cohort_shuffle(inner, cohort_size, seed + cohort);

#ifdef DEBUG_COHORT
  fprintf(stderr, "\nmultipoly_outer::cohort/inner::%lu/%lu\n", cohort, inner);
  fprintf(
    stderr,
    "multipoly_outer::shape/base/size/super::%lu/%lu/%lu/%lu\n",
    cohort_shape,
    cohort_size_base,
    cohort_size,
    super_size
  );
#endif

  id inv_inner = cohort_size - 1 - inner;

  id segment = acy_inv_quadsum(inv_inner, cohort_shape);
  id after = acy_quadsum(segment, cohort_shape);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_outer::segment/after::%lu/%lu\n",
    segment,
    after
  );
#endif

  id super_cohort = cohort / cohort_size_base;
  id section = (cohort % cohort_size_base) + (cohort_size_base - segment) - 1;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_outer::super_cohort/full_section::%lu/%lu\n",
    super_cohort,
    section
  );
#endif

  id in_segment = inv_inner - after;

  if (section >= cohort_size_base) {
    super_cohort += 1;
    section -= cohort_size_base;
  }

  id shuf = after + in_segment;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_outer::new_super/section/in_segment/shuffled::%lu/%lu/%lu/%lu\n",
    super_cohort,
    section,
    in_segment,
    shuf
  );
#endif

  id in_section = acy_rev_cohort_shuffle(shuf, cohort_size, seed + section);

  id result = acy_cohort_outer(
    super_cohort,
    section * cohort_size + in_section,
    super_size
  );

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multipoly_outer::inner/result::%lu/%lu\n\n",
    section * cohort_size + in_section,
    result
  );
#endif

  return result;
}

#endif // INCLUDE_COHORT_H
