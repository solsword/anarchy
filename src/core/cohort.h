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
static inline id myc_cohort(id outer, id cohort_size) {
  return outer / cohort_size;
}

// Identifies our within-cohort id.
static inline id myc_cohort_inner(id outer, id cohort_size) {
  return outer % cohort_size;
}

// Combines cohort and cohort_inner, returning both values via return
// parameters.
static inline void myc_cohort_and_inner(
  id outer,
  id cohort_size,
  id *r_cohort,
  id *r_inner
) {
  *r_cohort = myc_cohort(outer, cohort_size);
  *r_inner = myc_cohort_inner(outer, cohort_size);
}

// Find global ID from cohort & inner ID:
static inline id myc_cohort_outer(
  id cohort,
  id inner,
  id cohort_size
) {
  id cohort_start = cohort * cohort_size;
  return cohort_start + inner;
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

// Does a different cohort spin for even and odd items.
static inline id myc_cohort_mix(id inner, id cohort_size, id seed) {
  id even = inner - inner % 2;
  id target;
  if (inner % 2) {
    target = myc_cohort_spin(
      even/2,
      (cohort_size+(1-cohort_size%2))/2,
      seed + 464185
    );
    return 2 * target + 1;
  } else {
    target = myc_cohort_spin(even/2, (cohort_size+1)/2, seed + 1048239);
    return 2 * target;
  }
}

// Reverse
static inline id myc_rev_cohort_mix(id mixed, id cohort_size, id seed) {
  id even = mixed - mixed % 2;
  id target;
  if (mixed % 2) {
    target = myc_rev_cohort_spin(
      even/2,
      (cohort_size+(1-cohort_size%2))/2,
      seed + 464185
    );
    return 2 * target + 1;
  } else {
    target = myc_rev_cohort_spin(even/2, (cohort_size+1)/2, seed + 1048239);
    return 2 * target;
  }
}

// Spreads items out between a number of different regions.
#define MIN_REGION_SIZE 2
#define MAX_REGION_COUNT 16
// TODO: Don't send stuff out-of-range
static inline id myc_cohort_spread(id inner, id cohort_size, id seed) {
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
static inline id myc_rev_cohort_spread(id spread, id cohort_size, id seed) {
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
static inline id myc_cohort_upend(id inner, id cohort_size, id seed) {
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
static inline id myc_cohort_shuffle(id inner, id cohort_size, id seed) {
  id r = inner;
  seed ^= cohort_size/3;
  r = myc_cohort_spread(r, cohort_size, seed + 453);
  r = myc_cohort_mix(r, cohort_size, seed + 2891);
  r = myc_cohort_interleave(r, cohort_size);
  r = myc_cohort_spin(r, cohort_size, seed + 1982);
  r = myc_cohort_upend(r, cohort_size, seed + 47);
  r = myc_cohort_fold(r, cohort_size, seed + 837);
  r = myc_cohort_interleave(r, cohort_size);
  r = myc_cohort_flop(r, cohort_size, seed + 53);
  r = myc_cohort_fold(r, cohort_size, seed + 201);
  r = myc_cohort_mix(r, cohort_size, seed + 728);
  r = myc_cohort_spread(r, cohort_size, seed + 881);
  r = myc_cohort_interleave(r, cohort_size);
  r = myc_cohort_flop(r, cohort_size, seed + 192);
  r = myc_cohort_upend(r, cohort_size, seed + 794614);
  r = myc_cohort_spin(r, cohort_size, seed + 19);
  return r;
}

// Reverse
static inline id myc_rev_cohort_shuffle(id shuffled, id cohort_size, id seed) {
  id r = shuffled;
  seed ^= cohort_size/3;
  r = myc_rev_cohort_spin(r, cohort_size, seed + 19);
  r = myc_cohort_upend(r, cohort_size, seed + 794614);
  r = myc_cohort_flop(r, cohort_size, seed + 192);
  r = myc_rev_cohort_interleave(r, cohort_size);
  r = myc_rev_cohort_spread(r, cohort_size, seed + 881);
  r = myc_rev_cohort_mix(r, cohort_size, seed + 728);
  r = myc_rev_cohort_fold(r, cohort_size, seed + 201);
  r = myc_cohort_flop(r, cohort_size, seed + 53);
  r = myc_rev_cohort_interleave(r, cohort_size);
  r = myc_rev_cohort_fold(r, cohort_size, seed + 837);
  r = myc_cohort_upend(r, cohort_size, seed + 47);
  r = myc_rev_cohort_spin(r, cohort_size, seed + 1982);
  r = myc_rev_cohort_interleave(r, cohort_size);
  r = myc_rev_cohort_mix(r, cohort_size, seed + 2891);
  r = myc_rev_cohort_spread(r, cohort_size, seed + 453);
  return r;
}

// A cohort of the given size drawn from a double-wide segment of the outer
// region with 50% representation. Note that the inner indices of mixed cohorts
// are shuffled, but the bottom 1/2 indices always come earlier than the top
// 1/2 indices. Simply re-shuffle the inner results if you wish to have mixed
// indices (the biased indices are useful for some purposes).
// TODO: Guarantee that size isn't off-by-one?
static inline id myc_mixed_cohort(id outer, id cohort_size, id seed) {
  id strict_cohort = myc_cohort(outer, cohort_size);
  id strict_inner = myc_cohort_inner(outer, cohort_size);

  id shuf = myc_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);
  id lower = shuf < cohort_size/2;

  return (
     lower * (strict_cohort + 1)
  + !lower * (strict_cohort)
  );
}

// Gets the inner id for a mixed cohort (see above)
static inline id myc_mixed_cohort_inner(id outer, id cohort_size, id seed) {
  id strict_cohort = myc_cohort(outer, cohort_size);
  id strict_inner = myc_cohort_inner(outer, cohort_size);

  id shuf = myc_cohort_shuffle(strict_inner, cohort_size, seed + strict_cohort);

  return shuf;
}

// Combines myc_mixed_cohort and myc_mixed_cohort_inner, returning both values
// via return parameters. More efficient if you need both values.
static inline void myc_mixed_cohort_and_inner(
  id outer,
  id cohort_size,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id strict_cohort = myc_cohort(outer, cohort_size);
  id strict_inner = myc_cohort_inner(outer, cohort_size);

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
  return myc_cohort_outer(strict_cohort, unshuf, cohort_size);
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

  id strict_cohort = myc_cohort(outer, cohort_size);
  id strict_inner = myc_cohort_inner(outer, cohort_size);

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
  return myc_cohort_outer(strict_cohort, unshuf, cohort_size);
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
//   section_width indicates how wide each section is.
//
//   which indicates which section's cutoff we're interested in.
//
static inline id myc_exp_split(
  float shape,
  id section_count,
  id section_width,
  id which
) {
  if (shape < 0) {
    which = section_count - which - 1;
    shape = -shape;
  }
  return (id) (section_width * exp(-(which/(float)section_count)*-log(shape)));
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

  id strict_cohort = myc_cohort(outer, cohort_size);
  id strict_inner = myc_cohort_inner(outer, cohort_size);

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

  id section = strict_inner / resolution;
  id in_section = strict_inner % resolution;
  // Note: ID coherency between cohorts is impossible if we also want a smooth
  // distribution of cohort members throughout ID space (which is much more
  // important)
  id shuf = myc_cohort_shuffle(
    in_section,
    resolution,
    seed + section
  );
  id split = myc_exp_split(shape, section_count, resolution, section);
  id lower = shuf < split;

  // TODO: Type here?
  int adjust = !lower * (-1 + 2*(shape > 0));

#ifdef DEBUG_COHORT
  fprintf(stderr, "exp_cohort::adjust::%d\n\n", adjust);
#endif

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

  id split = myc_exp_split(shape, section_count, resolution, section);
  id lower = in_section < split;

  int adjust = !lower * (-1 + 2*(shape > 0));

  id strict_cohort = cohort - adjust;

  id unshuf = myc_rev_cohort_shuffle(in_section, resolution, seed + section);

  id strict_inner = (section * resolution) + unshuf;

  return myc_cohort_outer(strict_cohort, strict_inner, cohort_size);
}

// Works like myc_exp_split but computes multiple layered splits, using the
// additional layer and n_layers argument to select a layer. Note that section
// indices (which) are relative to cohort -1, so a section index of
// section_count is the 0th section of the current cohort.
static inline id myc_multi_exp_split(
  float shape,
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
  return myc_exp_split(
    shape,
    section_count,
    section_width,
    which + adjust
  );
}

// Looks up which layer we're in using myc_multi_exp_split. Note that the layer
// ranges from 0 to n_layers*2+1, and the sections are indexed starting from
// the beginning of the previous cohort.
static inline id myc_multi_exp_get_layer(
  id in_section,
  float shape,
  id section_count,
  id section_width,
  id which,
  id n_layers
) {
  id layer = 0;
  id last_split = 0;
  id split;
  do {
    split = myc_multi_exp_split(
      shape,
      section_count,
      section_width,
      which,
      layer,
      n_layers
    );
    if (split < last_split) {
      layer += 1;
      break;
    }
    last_split = split;
    layer += 1;
  } while (in_section >= split && layer < n_layers*2 + 1);
  return layer - 1;
}

// Computes the top and bottom limits of the given layer in the given section.
static inline void myc_multi_exp_limits(
  float shape,
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
  id bottom = myc_multi_exp_split(
    shape,
    section_count,
    section_width,
    which,
    layer - 1,
    n_layers
  );
  id top = myc_multi_exp_split(
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

// Takes an inner index (inner) within a cohort of the given size (cohort_size)
// And returns (via return parameters r_layer and r_inner) the layer to which
// that index belongs and which index it has within that layer.
static inline void myc_multiexp_layer_and_inner(
  id inner,
  id cohort_size,
  float shape,
  id section_count,
  id section_width,
  id n_layers,
  id seed,
  id *r_layer,
  id *r_inner
) {
  id leftovers = cohort_size - (section_count * section_width);
  id section = inner / section_width;
  id in_section = inner % section_width;
  id full_section = section + section_count;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\nmultiexp_layer::inner/shape/size::%lu/%.3f/%lu\n",
    inner,
    shape,
    cohort_size
  );
  fprintf(
    stderr,
    "multiexp_layer::section count/width/n_layers/leftovers::%lu/%lu/%lu/%lu\n",
    section_count,
    section_width,
    n_layers,
    leftovers
  );
  fprintf(
    stderr,
    "multiexp_layer::section/full/in_section::%lu/%lu/%lu\n",
    section,
    full_section,
    in_section
  );
#endif

  id shuf;
  if (section < section_count) {
    shuf = myc_cohort_shuffle(
      in_section,
      section_width,
      seed + section
    );
  } else {
    if (leftovers > 1) {
      shuf = myc_cohort_shuffle(
        in_section,
        leftovers,
        seed + section
      );
    } else {
      shuf = in_section;
    }
  }

#ifdef DEBUG_COHORT
  fprintf(stderr, "multiexp_layer::shuf::%lu\nSplits: ", shuf);
#endif

  // Find layer and relevant splits
  id layer = 0;
  id last_split = 0;
  id split;
  do {
    split = myc_multi_exp_split(
      shape,
      section_count,
      section_width,
      full_section,
      layer,
      n_layers
    );
    if (split < last_split) {
      layer += 1;
#ifdef DEBUG_COHORT
      fprintf(stderr, "X\nSplit %lu < %lu", split, last_split);
#endif
      break;
    }
#ifdef DEBUG_COHORT
    fprintf(stderr, "%lu %lu--%lu\n", layer, last_split, split);
    /*
    for (id i = 0; i < split - last_split; ++i) {
      fputc(0x30 + layer, stderr);
    }
    */
#endif
    last_split = split;
    layer += 1;
  } while (shuf >= split && layer < n_layers*2 + 1);
  layer -= 1;
#ifdef DEBUG_COHORT
  fputc('\n', stderr);
#endif

#ifdef DEBUG_COHORT
  fprintf(stderr, "multiexp_layer::layer::%lu\n", layer);
#endif

  // Figure out position within layer (in this section):
  id within_layer;
  if (layer > 0) {
    within_layer = shuf - myc_multi_exp_split(
      shape,
      section_count,
      section_width,
      full_section,
      layer-1,
      n_layers
    );
  } else {
    within_layer = shuf;
  }

#ifdef DEBUG_COHORT
  fprintf(stderr, "multiexp_layer::within_layer::%lu\n", within_layer);
#endif

  // Figure out how many other indices precede us:
  // TODO: What if this rounds?
  id layer_width = section_count / n_layers; // in sections
  id layer_origin_section = layer_width * layer;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_layer::layer width/origin::%lu/%lu\n",
    layer_width,
    layer_origin_section
  );
#endif

  id before = 0; // number of ids before here
  id bottom, top;
  for ( // iterate over all previous sections within this layer
    id previous_section = layer_origin_section - layer_width;
    previous_section < full_section;
    ++previous_section
  ) {
    myc_multi_exp_limits(
      shape,
      section_count,
      section_width,
      previous_section,
      layer,
      n_layers,
      &bottom,
      &top
    );
#ifdef DEBUG_COHORT
    fprintf(
      stderr,
      "BefCalc [%lu]: %lu--%lu (%lu → %lu)\n",
      previous_section,
      bottom, top,
      before, before + (top - bottom)
    );
#endif
    before += top - bottom;
  }

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_layer::before/result::%lu/%lu\n\n",
    before,
    before + within_layer
  );
#endif

  // Results:
  *r_layer = layer;
  *r_inner = before + within_layer;
}

// Inverse of myc_multiexp_layer_and_inner
static inline id myc_multiexp_layer_outer(
  id layer,
  id within_layer,
  id cohort_size,
  float shape,
  id section_count,
  id section_width,
  id n_layers,
  id seed
) {
  id leftovers = cohort_size - (section_count * section_width);

  // Iterate to find within-section/layer index:
  id layer_width = section_count / n_layers; // in sections
  id layer_origin_section = layer_width * layer;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\nmultiexp_layer_outer::layer width/origin::%lu/%lu\n",
    layer_width,
    layer_origin_section
  );
#endif

  id full_section = layer_origin_section - layer_width - 1;
  id span = 0;
  id bottom, top;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_layer_outer::layer/inner/shape/size::%lu/%lu/%.3f/%lu\n",
    layer,
    within_layer,
    shape,
    cohort_size
  );
  fprintf(
    stderr,
    "multiexp_layer_outer::section count/width/n_layers::%lu/%lu/%lu\n",
    section_count,
    section_width,
    n_layers
  );
  fprintf(
    stderr,
    "multiexp_layer_outer::leftovers/layer_origin::%lu/%lu\n",
    leftovers,
    layer_origin_section
  );
#endif

  id adjust = 0;
  do {
    adjust += span;
    full_section += 1;
    myc_multi_exp_limits(
      shape,
      section_count,
      section_width,
      full_section,
      layer,
      n_layers,
      &bottom,
      &top
    );
#ifdef DEBUG_COHORT
    fprintf(
      stderr,
      "AdjCalc [%lu]: %lu--%lu (%lu → %lu)\n",
      full_section,
      bottom, top,
      adjust, adjust + (top - bottom)
    );
#endif
    span = top - bottom;
  } while (
    within_layer - adjust >= span
 && (
      full_section - (layer_origin_section - layer_width)
    < (section_count + layer_width)
    )
  );
  id adjusted = within_layer - adjust;
  if (
    full_section < section_count
  ||
    adjusted > span
  ) { // out-of-bounds (too early or too far)
#ifdef DEBUG_COHORT
    fprintf(
      stderr,
      "\nInner out-of-bounds in layer: "
        "section %lu layer %lu [%lu--%lu] inner %lu\n"
        "  Adjusted/span: %lu/%lu",
      full_section,
      layer,
      layer_origin_section,
      layer_origin_section + section_count + layer_width,
      within_layer,
      adjusted,
      span
    );
#endif
    return NONE;
  }
  // adjusted is now within section & layer
  // adjust section to fall within second cohort:
  id section = full_section - section_count;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_layer_outer::full_section/section::%lu/%lu\n",
    full_section,
    section
  );
#endif

  // bottom/top remain those within correct section:
  id within_section = bottom + adjusted;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_layer_outer::span/adjusted/within_section::%lu/%lu/%lu\n",
    span,
    adjusted,
    within_section
  );
#endif

  // unshuffle within section:
  id unshuf;
  if (section < section_count) {
    unshuf = myc_rev_cohort_shuffle(
      within_section,
      section_width,
      seed + section
    );
  } else {
    if (leftovers > 1) {
      unshuf = myc_rev_cohort_shuffle(
        within_section,
        leftovers,
        seed + section
      );
    } else {
      unshuf = within_section;
    }
  }

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_layer_outer::unshuffled/result::%lu/%lu\n\n",
    unshuf,
    section * section_width + unshuf
  );
#endif

  // return index within layered cohort:
  return section * section_width + unshuf;
}

// Works like myc_exp_cohort_and_inner but instead of slicing each cohort into
// two parts, it slices each cohort into multiple parts, and distributes them
// nearby. This can give a much smoother distribution.
static inline void myc_multiexp_cohort_and_inner(
  id outer,
  float shape,
  id cohort_size,
  id n_layers,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id full_size = cohort_size * n_layers;
  id resolution = EXP_SECTION_RESOLUTION;
  id section_count = full_size / resolution;
  if (section_count < MIN_SECTION_COUNT) {
    resolution = full_size / MIN_SECTION_COUNT;
    if (resolution < MIN_SECTION_RESOLUTION) {
      resolution = MIN_SECTION_RESOLUTION;
    }
    section_count = full_size / resolution;
  }

  id strict_cohort;
  id strict_inner;
  myc_cohort_and_inner(outer, full_size, &strict_cohort, &strict_inner);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\nmultiexp_cohort::outer/shape/size/full_size::%lu/%.3f/%lu/%lu\n",
    outer,
    shape,
    cohort_size,
    full_size
  );
  fprintf(
    stderr,
    "multiexp_cohort::strict_cohort/inner::%lu/%lu\n",
    strict_cohort,
    strict_inner
  );
#endif

  id layer, layer_inner;
  myc_multiexp_layer_and_inner(
    strict_inner,
    full_size,
    shape,
    section_count,
    resolution,
    n_layers,
    seed,
    &layer,
    &layer_inner
  );

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_cohort::sections/resolution::%lu/%lu\n",
    section_count,
    resolution
  );
  fprintf(
    stderr,
    "multiexp_cohort::layer/inner::%lu/%lu\n",
    layer,
    layer_inner
  );
#endif

  id full_n_layers = n_layers * 2 + 1;
  id adjusted_cohort = strict_cohort * full_n_layers + layer;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "multiexp_cohort::adjusted_cohort::%lu\n\n",
    adjusted_cohort
  );
#endif

  if (adjusted_cohort < strict_cohort) { // overflow
    *r_cohort = NONE;
    *r_inner = NONE;
    return;
  }
  *r_cohort = adjusted_cohort;
  *r_inner = layer_inner;
}

static inline id myc_multiexp_cohort_outer(
  id cohort,
  id inner,
  float shape,
  id cohort_size,
  id n_layers,
  id seed
) {
  id full_size = cohort_size * n_layers;
  id resolution = EXP_SECTION_RESOLUTION;
  id section_count = full_size / resolution;
  if (section_count < MIN_SECTION_COUNT) {
    resolution = full_size / MIN_SECTION_COUNT;
    if (resolution < MIN_SECTION_RESOLUTION) {
      resolution = MIN_SECTION_RESOLUTION;
    }
    section_count = full_size / resolution;
  }

  // Back out the strict cohort and layer:
  id full_n_layers = n_layers * 2 + 1;
  id strict_cohort = cohort / full_n_layers;
  id layer = cohort % full_n_layers;

  id strict_inner = myc_multiexp_layer_outer(
    layer,
    inner,
    full_size,
    shape,
    section_count,
    resolution,
    n_layers,
    seed
  );

  return myc_cohort_outer(strict_cohort, strict_inner, full_size);
}

#endif // INCLUDE_COHORT_H
