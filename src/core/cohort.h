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

// Computes the sum from k=1 to n of (1 + k*shape), which folds down to:
//
//   (shape * (n - 1) + 2) * (shape / 2)
//
static inline id quadsum(id n, id shape) {
  return ((shape * (n - 1) + 2) * n) / 2;
}

// For a given cohort shape and desired cohort size, computes the nearest
// workable size for acy_multipoly_cohort_and_inner/acy_multipoly_cohort_outer
// and returns (via return parameters) the computed size and the appropriate
// base to use as an argument to get that size.
static inline void acy_multipoly_nearest_cohort_size(
  id cohort_shape
  id cohort_size,
  id *r_nearest,
  id *r_base
) {
  // TODO: HERE!
}

// Works like acy_multiexp_cohort_and_inner but restricts possible cohort sizes
// and uses a merely polynomial distribution to ensure inner cohort ID
// completeness & continuity. See acy_multipoly_nearest_cohort_size above for
// details on the cohort_size_base parameter.
// TODO: Find a telescoping exponential sum!!
static inline void acy_multipoly_cohort_and_inner(
  id outer,
  double shape,
  id cohort_size_base,
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

#endif // INCLUDE_COHORT_H
