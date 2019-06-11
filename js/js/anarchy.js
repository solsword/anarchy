// anarchy.js
// Reversible chaos library.

define([], function() {
  // Note: anarchy.js operates using 32-bit integer values to remain
  // dependency-free. This breaks full compatibility with the C library, which
  // uses 64-bit integers for obvious reasons. Javascript does not support
  // 64-bit integers at this time (Number.MAX_SAFE_INTEGER is 2^53-1), and in
  // particular, bitwise operations only work on 32-bit integers.
  var ID_BITS = 32;
  var ID_BYTES = 4;

  function posmod(x, y) {
    // Modulus that's always positive:
    return ((x % y) + y) % y;
  }

  // A string hashing function.
  // See: https://stackoverflow.com/questions/7616461/generate-a-hash-from-string-in-javascript-jquery
  function hash_string(s) {
    var hash = 0, i, chr;
    if (s.length === 0) {
      return hash;
    }
    for (i = 0; i < s.length; i++) {
      chr = s.charCodeAt(i);
      hash = ((hash << 5) - hash) + chr;
      hash |= 0; // Convert to 32bit integer
    }
    return hash;
  }

  function mask(bits) {
    // Creates a mask with the given number of 1 bits.
    // Avoids shift operator because of 32-bit limit, but watch out for
    // floating point behavior for bit amounts greater than ~52 (see
    // Number.MAX_SAFE_INTEGER).
    return Math.pow(2, bits) - 1;
  }

  function byte_mask(n) {
    // Returns a mask that covers just the nth byte (zero-indexed), starting
    // from the least-significant digits. Starts to break down when
    // Number.MAX_SAFE_INTEGER is exceeded (byte 6).
    return Math.pow(2, n*8) * 255;
  }

  function swirl(x, distance) {
    // Circular bit shift; distance is capped at 3/4 of ID_BITS
    distance = posmod(distance, Math.floor(3 * ID_BITS / 4));
    var m = mask(distance);
    var fall_off = x & m;
    var shift_by = (ID_BITS - distance);
    return (
      (x >>> distance)
    | (fall_off << shift_by)
    ) >>> 0;
  }

  function rev_swirl(x, distance) {
    // Inverse circular shift (see above).
    distance = posmod(distance, Math.floor(3 * ID_BITS / 4));
    var m = mask(distance);
    var m = mask(distance);
    var fall_off = x & (m << (ID_BITS - distance));
    var shift_by = (ID_BITS - distance);
    return (
      (x << distance)
    | (fall_off >>> shift_by)
    ) >>> 0;
  }

  function fold(x, where) {
    // Folds lower bits into upper bits using xor. 'where' is restricted to
    // fall between 1/4 and 1/2 of ID_BITS.
    var quarter = Math.floor(ID_BITS / 4)
    var where = posmod(where, quarter) + quarter;
    var m = mask(where);
    var lower = x & m;
    var shift_by = ID_BITS - where;
    return (x ^ (lower << shift_by)) >>> 0;
  }
  // fold is its own inverse.

  var FLOP_MASK = 0xf0f0f0f0;

  function flop(x) {
    // Flops each 1/2 byte with the adjacent 1/2 byte.
    var left = x & FLOP_MASK;
    var right = x & ~FLOP_MASK;
    return ((right << 4) | (left >>> 4)) >>> 0;
  }
  // flop is its own inverse.

  function scramble(x) {
    // Implements a reversible linear-feedback-shift-register-like operation.
    var trigger = x & 0x80200003;
    var r = swirl(x, 1);
    if (trigger) {
      r ^= 0x03040610;
    }
    return r >>> 0;
  }

  function rev_scramble(x) {
    // Inverse of scramble (see above).
    var pr = rev_swirl(x, 1);
    var trigger = pr & 0x80200003;
    if (trigger) {
      // pr ^= rev_swirl(0x03040610, 1);
      pr ^= 0x06080c20;
    }
    return pr >>> 0;
  }

  function scramble_seed(s) {
    // Scrambles a seed value to help separate RNG sequences generated from
    // sequential seeds.

    s = s >>> 0;
    s = ((s + 1) * (3 + posmod(s, 23))) >>> 0;
    s = fold(s, 11) >>> 0; // prime
    s = scramble(s) >>> 0;
    s = swirl(s, s + 23) >>> 0; // prime
    s = scramble(s) >>> 0;
    s ^= posmod(s, 153) * scramble(s);
    s = s >>> 0;

    return s;
  }

  function prng(x, seed) {
    // A simple reversible pseudo-random number generator.

    // seed scrambling:
    seed = scramble_seed(seed);

    // value scrambling:
    x ^= seed;
    x = fold(x, seed + 17); // prime
    x = flop(x);
    x = swirl(x, seed + 37); // prime
    x = fold(x, seed + 89); // prime
    x = swirl(x, seed + 107); // prime
    x = scramble(x);
    return x >>> 0;
  }

  function rev_prng(x, seed) {
    // Inverse of prng (see above).

    // seed scrambling:
    seed = scramble_seed(seed);

    // value unscrambling:
    x = rev_scramble(x);
    x = rev_swirl(x, seed + 107); // prime
    x = fold(x, seed + 89); // prime
    x = rev_swirl(x, seed + 37); // prime
    x = flop(x);
    x = fold(x, seed + 17); // prime
    x ^= seed;
    return x >>> 0;
  }

  function lfsr(x) {
    // Implements a max-cycle-length 32-bit linear-feedback-shift-register.
    // See: https://en.wikipedia.org/wiki/Linear-feedback_shift_register
    // Note that this is NOT reversible!
    var lsb = x & 1;
    var r = x >>> 1;
    if (lsb) {
      r ^= 0x80200003; // 32, 22, 2, 1
    }
    return r;
  }

  function udist(x) {
    // Generates a random number between 0 and 1 given a seed value.
    var ux = lfsr(x >>> 0);
    var sc = (ux ^ (ux << 16)) >>> 0;
    return (sc % 2147483659) / 2147483659; // prime near 2^31
  }

  function idist(x, start, end) {
    // Even distribution over the given integer range, including the lower end
    // but excluding the higher end (even if the lower end is given second).
    // Distribution bias is about one part in (range/2^31).
    return Math.floor(udist(x) * (end - start)) + start;
  }

  function expdist(x) {
    // Generates a number from an exponential distribution with mean 0.5 given
    // a seed. See:
    // https://math.stackexchange.com/questions/28004/random-exponential-like-distribution
    var u = udist(x);
    return -Math.log(1 - u)/0.5;
  }

  function cohort(outer, cohort_size) {
    // Computes cohort number for the given outer index and cohort size.
    return Math.floor(outer / cohort_size) >>> 0;
  }

  function cohort_inner(outer, cohort_size) {
    // Computes within-cohort index for the given outer index and cohorts of
    // the given size.
    return posmod(outer, cohort_size) >>> 0;
  }

  function cohort_and_inner(outer, cohort_size) {
    // Returns an array containing both the cohort number and inner index for
    // the given outer index and cohort size.
    return [ cohort(outer, cohort_size), cohort_inner(outer, cohort_size) ];
  }

  function cohort_outer(cohort, inner, cohort_size) {
    // Inverse of cohort_and_inner; computes the outer index from a cohort
    // number and inner index.
    return (cohort_size * cohort + inner) >>> 0;
  }

  function cohort_interleave(inner, cohort_size) {
    // Interleaves cohort members by folding the top half into the bottom half.
    if (inner < Math.floor((cohort_size+1)/2)) {
      return (inner * 2) >>> 0;
    } else {
      return (((cohort_size - 1 - inner) * 2) + 1) >>> 0;
    }
  }

  function rev_cohort_interleave(inner, cohort_size) {
    // Inverse interleave (see above).
    if (posmod(inner, 2)) {
      return cohort_size - 1 - Math.floor(inner/2);
    } else {
      return Math.floor(inner/2);
    }
  }

  function cohort_fold(inner, cohort_size, seed) {
    // Folds items past an arbitrary split point (in the second half of the
    // cohort) into the middle of the cohort. The split will always leave an
    // odd number at the end.
    var half = Math.floor(cohort_size / 2);
    var quarter = Math.floor(cohort_size / 4);
    var split = half;
    if (quarter > 0) {
      split += posmod(seed, quarter);
    }
    var after = cohort_size - split;
    split += posmod(after + 1, 2); // force an odd split point
    after = cohort_size - split;

    var fold_to = half - Math.floor(after / 2);

    if (inner < fold_to) { // first region
      return inner >>> 0;
    } else if (inner < split) { // second region
      return (inner + after) >>> 0; // push out past fold region
    } else { // fold region
      return (inner - split + fold_to) >>> 0;
    }
  }

  function rev_cohort_fold(inner, cohort_size, seed) {
    // Inverse fold (see above).
    var half = Math.floor(cohort_size / 2);
    var quarter = Math.floor(cohort_size / 4);
    var split = half;
    if (quarter > 0) {
      split += posmod(seed, quarter);
    }
    var after = cohort_size - split;
    split += posmod((after + 1), 2); // force an odd split point
    after = cohort_size - split;

    var fold_to = half - Math.floor(after / 2);

    if (inner < fold_to) { // first region
      return inner >>> 0;
    } else if (inner < fold_to + after) { // second region
      return (inner - fold_to + split) >>> 0;
    } else {
      return (inner - after) >>> 0;
    }
  }

  function cohort_spin(inner, cohort_size, seed) {
    // Applies a circular offset
    return posmod((inner + seed), cohort_size) >>> 0;
  }

  function rev_cohort_spin(inner, cohort_size, seed) {
    // Inverse spin (see above).
    return posmod(
      inner + (cohort_size - posmod(seed, cohort_size)),
      cohort_size
    ) >>> 0;
  }

  function cohort_flop(inner, cohort_size, seed) {
    // Flops sections (with seeded sizes) with their neighbors.
    var limit = Math.floor(cohort_size / 8);
    if (limit < 4) {
      limit += 4;
    }
    var size = posmod(seed, limit) + 2;
    var which = Math.floor(inner / size);
    var local = posmod(inner, size);

    var result = 0;
    if (posmod(which, 2)) {
      result = (which - 1) * size + local;
    } else {
      result = (which + 1) * size + local;
    }

    if (result >= cohort_size) { // don't flop out of the cohort
      return inner >>> 0;
    } else {
      return result >>> 0;
    }
  }
  // flop is its own inverse

  function cohort_mix(inner, cohort_size, seed) {
    // Applies a spin to both even and odd items with different seeds.
    var even = inner - posmod(inner, 2);
    var target = 0;
    if (posmod(inner, 2)) {
      target = cohort_spin(
        Math.floor(even / 2),
        Math.floor(cohort_size / 2),
        seed + 464185
      );
      return (2 * target + 1) >>> 0;
    } else {
      target = cohort_spin(
        Math.floor(even / 2),
        Math.floor((cohort_size + 1) / 2),
        seed + 1048239
      );
      return (2 * target) >>> 0;
    }
  }

  function rev_cohort_mix(inner, cohort_size, seed) {
    // Inverse mix (see above).
    var even = inner - posmod(inner, 2);
    var target = 0;
    if (posmod(inner, 2)) {
      target = rev_cohort_spin(
        Math.floor(even / 2),
        Math.floor(cohort_size / 2),
        seed + 464185
      );
      return (2 * target + 1) >>> 0;
    } else {
      target = rev_cohort_spin(
        Math.floor(even / 2),
        Math.floor((cohort_size + 1) / 2),
        seed + 1048239
      );
      return (2 * target) >>> 0;
    }
  }

  var MIN_REGION_SIZE = 2;
  var MAX_REGION_COUNT = 16;

  function cohort_spread(inner, cohort_size, seed) {
    // Spreads items out between a random number of different regions within
    // the cohort.
    var min_regions = 2;
    if (cohort_size < 2 * MIN_REGION_SIZE) {
      min_regions = 1;
    }
    var max_regions = 1 + Math.floor(cohort_size / MIN_REGION_SIZE);
    var regions = (
      min_regions + posmod(
        posmod(seed, (1 + (max_regions - min_regions))),
        MAX_REGION_COUNT
      )
    );
    var region_size = Math.floor(cohort_size / regions);
    var leftovers = cohort_size - (regions * region_size);

    var region = posmod(inner, regions);
    var index = Math.floor(inner / regions);
    if (index < region_size) { // non-leftovers
      return (region * region_size + index + leftovers) >>> 0;
    } else { // leftovers go at the front:
      return (inner - regions * region_size) >>> 0;
    }
  }

  function rev_cohort_spread(inner, cohort_size, seed) {
    // Inverse spread (see above).
    var min_regions = 2;
    if (cohort_size < 2 * MIN_REGION_SIZE) {
      min_regions = 1;
    }
    var max_regions = 1 + Math.floor(cohort_size / MIN_REGION_SIZE);
    var regions = (
      min_regions + posmod(
        posmod(seed, (1 + (max_regions - min_regions))),
        MAX_REGION_COUNT
      )
    );

    var region_size = Math.floor(cohort_size / regions);
    var leftovers = cohort_size - (regions * region_size);

    var index = Math.floor((inner - leftovers) / region_size);
    var region = posmod((inner - leftovers), region_size);

    if (inner < leftovers) { // leftovers back to the end:
      return (regions * region_size + inner) >>> 0;
    } else {
      return (region * regions + index) >>> 0;
    }
  }

  function cohort_upend(inner, cohort_size, seed) {
    // Reverses ordering within each of several fragments.
    var min_regions = 2;
    if (cohort_size < 2 * MIN_REGION_SIZE) {
      min_regions = 1;
    }
    var max_regions = 1 + Math.floor(cohort_size / MIN_REGION_SIZE);
    var regions = (
      min_regions + posmod(
        posmod(seed, (1 + (max_regions - min_regions))),
        MAX_REGION_COUNT
      )
    );
    var region_size = Math.floor(cohort_size / regions);

    var region = Math.floor(inner / region_size);
    var index = posmod(inner, region_size);

    var result = (region * region_size) + (region_size - 1 - index);

    if (result < cohort_size) {
      return result >>> 0;
    } else {
      return inner >>> 0;
    }
  }
  // Upend is its own inverse.

  function cohort_shuffle(inner, cohort_size, seed) {
    // Compose a bunch of the above functions to perform a nice thorough
    // shuffle within a cohort.
    var r = inner;
    seed = seed ^ cohort_size;
    r = cohort_spread(r, cohort_size, seed + 457); // prime
    r = cohort_mix(r, cohort_size, seed + 2897); // prime
    r = cohort_interleave(r, cohort_size);
    r = cohort_spin(r, cohort_size, seed + 1987); // prime
    r = cohort_upend(r, cohort_size, seed + 47); // prime
    r = cohort_fold(r, cohort_size, seed + 839); // prime
    r = cohort_interleave(r, cohort_size);
    r = cohort_flop(r, cohort_size, seed + 53); // prime
    r = cohort_fold(r, cohort_size, seed + 211); // prime
    r = cohort_mix(r, cohort_size, seed + 733); // prime
    r = cohort_spread(r, cohort_size, seed + 881); // prime
    r = cohort_interleave(r, cohort_size);
    r = cohort_flop(r, cohort_size, seed + 193); // prime
    r = cohort_upend(r, cohort_size, seed + 794641); // prime
    r = cohort_spin(r, cohort_size, seed + 19); // prime
    return r;
  }

  function rev_cohort_shuffle(inner, cohort_size, seed) {
    // Inverse shuffle (see above).
    var r = inner;
    seed = seed ^ cohort_size;
    r = rev_cohort_spin(r, cohort_size, seed + 19); // prime
    r = cohort_upend(r, cohort_size, seed + 794641); // prime
    r = cohort_flop(r, cohort_size, seed + 193); // prime
    r = rev_cohort_interleave(r, cohort_size);
    r = rev_cohort_spread(r, cohort_size, seed + 881); // prime
    r = rev_cohort_mix(r, cohort_size, seed + 733); // prime
    r = rev_cohort_fold(r, cohort_size, seed + 211); // prime
    r = cohort_flop(r, cohort_size, seed + 53); // prime
    r = rev_cohort_interleave(r, cohort_size);
    r = rev_cohort_fold(r, cohort_size, seed + 839); // prime
    r = cohort_upend(r, cohort_size, seed + 47); // prime
    r = rev_cohort_spin(r, cohort_size, seed + 1987); // prime
    r = rev_cohort_interleave(r, cohort_size);
    r = rev_cohort_mix(r, cohort_size, seed + 2897); // prime
    r = rev_cohort_spread(r, cohort_size, seed + 457); // prime
    return r;
  }

  function distribution_spilt_point(
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
  ) {
    // Implements common distribution functionality: given a total item count,
    // a segment count and per-segment capacity, and a roughness value and
    // seed, computes the split point for the items as well as the halfway
    // index for the segments.

    // how the segments are divided:
    var first_half = Math.floor(n_segments / 2);

    // compute min/max split points according to roughness:
    var half = Math.floor(total / 2);
    var split_min = Math.floor(half - half * roughness);
    var split_max = Math.floor(half + (total - half) * roughness);

    // adjust for capacity limits:
    if ((total - split_min) > segment_capacity * (n_segments - first_half)) {
      split_min = total - (segment_capacity * (n_segments - first_half));
    }

    if (split_max > segment_capacity * first_half) {
      split_max = segment_capacity * first_half;
    }

    // compute a random split point:
    var split = half;
    if (split_min >= split_max) {
      split = split_min;
    } else {
      split = split_min + posmod(
        prng(total ^ prng(seed)),
        (split_max - split_min)
      )
    }

    return [split, first_half];
  }

  function distribution_portion(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
  ) {
    // Given that 'total' items are to be distributed evenly among 'n_segment'
    // segments each with at most 'segment_capacity' items and we're in segment
    // 'segment' of those, computes how many items are in this segment. The
    // 'roughness' argument should be a number between 0 and 1 indicating how
    // even the distribution is: 0 indicates a perfectly even distribution,
    // while 1 indicates a perfectly random distribution. Does work
    // proportional to the log of the number of segments.
    //
    // Note that segment_capacity * n_segments should be > total.

    // base case
    if (n_segments == 1) {
      return total;
    }

    // compute split point:
    split = distribution_spilt_point(
      total,
      n_segments,
      segment_capacity,
      roughness,
      seed
    );

    // call ourselves recursively:
    if (segment < split[1]) {
      return distribution_portion(
        segment,
        split[0],
        split[1],
        segment_capacity,
        roughness,
        seed
      );
    } else {
      return distribution_portion(
        segment - split[1],
        total - split[0],
        n_segments - split[1],
        segment_capacity,
        roughness,
        seed
      );
    }
  }

  function distribution_prior_sum(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
  ) {
    // Does similar math to the distribution_portion function above, but
    // instead of returning the number of items in the given segment, it
    // returns the number of items in all segments before the given segment.
    // Only does work proportional to the log of the number of segments.

    // base case
    if (n_segments == 1) {
      return 0; // nothing prior
    }

    var first_half = Math.floor(n_segments / 2);

    // compute split point:
    split = distribution_spilt_point(
      total,
      n_segments,
      segment_capacity,
      roughness,
      seed
    );

    // call ourselves recursively:
    if (segment < first_half) {
      return distribution_prior_sum(
        segment, // segment
        split[0], // total
        split[1], // n_segments
        segment_capacity, // capacity
        roughness, // roughness
        seed // seed
      );
    } else {
      return split[0] + distribution_prior_sum(
        segment - split[1],
        total - split[0],
        n_segments - split[1],
        segment_capacity,
        roughness,
        seed
      );
    }
  }

  function distribution_segment(
    index,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
  ) {
    // Computes the segment number in which a certain item appears (one of the
    // 'total' items distributed between segments; see distribution_portion
    // above). Requires work proportional to the log of the number of segments.

    // base case
    if (n_segments == 1) {
      return 0; // we are in the only segment there is
    }

    // compute split point:
    split = distribution_spilt_point(
      total,
      n_segments,
      segment_capacity,
      roughness,
      seed
    );

    // call ourselves recursively:
    if (index < split) {
      return distribution_segment(
        index,
        split[0],
        split[1],
        segment_capacity,
        roughness,
        seed
      );
    } else {
      return split[1] + distribution_segment(
        index - split[0],
        total - split[0],
        n_segments - split[1],
        segment_capacity,
        roughness,
        seed
      );
    }
  }

  function max_smaller(value, sumtable) {
    // Uses binary search to find and return the index of the largest sum in
    // the given sumtable that's smaller than the given value. Works in time
    // proportional to the logarithm of the sumtable size. Returns -1 if there
    // is no entry in the sumtable smaller than the given value.
    var from = 0;
    var to = sumtable.length;
    var where = 0;
    while (to - from > 2) {
      where = Math.floor((to - from)/2);
      if (mpsums[where] >= value) {
        to = where;
      } else {
        from = where;
      }
    }
    if (to - from == 1 && sumtable[from] < value) {
      return from;
    } else {
      for (var i = from; i < to - 1; ++i) {
        if (sumtable[i] < value && sumtable[i+1] >= value) {
          return i;
        }
      }
    }
    // no entry is smaller than the given value:
    return -1;
  }

  return {
    "posmod": posmod,
    "hash_string": hash_string,
    "mask": mask,
    "byte_mask": byte_mask,
    "swirl": swirl,
    "rev_swirl": rev_swirl,
    "fold": fold,
    "flop": flop,
    "scramble": scramble,
    "rev_scramble": rev_scramble,

    "prng": prng,
    "rev_prng": rev_prng,

    "lfsr": lfsr,

    "udist": udist,
    "idist": idist,
    "expdist": expdist,

    "cohort": cohort,
    "cohort_inner": cohort_inner,
    "cohort_and_inner": cohort_and_inner,
    "cohort_outer": cohort_outer,

    "cohort_interleave": cohort_interleave,
    "rev_cohort_interleave": rev_cohort_interleave,
    "cohort_fold": cohort_fold,
    "rev_cohort_fold": rev_cohort_fold,
    "cohort_spin": cohort_spin,
    "rev_cohort_spin": rev_cohort_spin,
    "cohort_flop": cohort_flop,
    "cohort_mix": cohort_mix,
    "rev_cohort_mix": rev_cohort_mix,
    "cohort_spread": cohort_spread,
    "rev_cohort_spread": rev_cohort_spread,
    "cohort_upend": cohort_upend,
    "cohort_shuffle": cohort_shuffle,
    "rev_cohort_shuffle": rev_cohort_shuffle,

    "distribution_spilt_point": distribution_spilt_point,
    "distribution_portion": distribution_portion,
    "distribution_prior_sum": distribution_prior_sum,
    "distribution_segment": distribution_segment,

    "max_smaller": max_smaller,
  };
});
