using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Anarchy : MonoBehaviour
{
  public void Start() {}
  public void Update() {}

  // TODO: TESTS!

  public static int ID_BITS = 64;
  public static int ID_BYTES = 8; 

  // Modulus that's always positive:
  public ulong posmod(ulong x, ulong y) { return ((x % y) + y) % y; }
  // Polymorphic variants:
  public int posmod(int x, int y) { return ((x % y) + y) % y; }
  public ulong posmod(ulong x, int y) {
    ulong yy = (ulong) y;
    return ((x % yy) + yy) % yy;
  }


  public ulong mask(int bits) {
    // Creates a mask with the given number of 1 bits.
    return (1ul << bits) - 1ul;
  }

  public ulong byte_mask(int b) {
    // Create a mask that covers a particular byte
    return (255ul) << (b * 8);
  }

  public ulong swirl(ulong x, ulong distance) {
    // Circular bit shift; distance is capped at 3/4 of ID_BITS
    distance = posmod(distance, (ulong) (3 * Anarchy.ID_BITS) / 4);
    ulong m = mask((int) distance);
    ulong fall_off = x & m;
    int shift_by = (Anarchy.ID_BITS - (int) distance);
    return (
      (x >> (int) distance)
    | (fall_off << shift_by)
    );
  }

  public ulong rev_swirl(ulong x, ulong distance) {
    // Inverse circular shift (see above).
    distance = posmod(distance, (ulong) (3 * Anarchy.ID_BITS) / 4);
    ulong m = mask((int) distance);
    ulong fall_off = x & (m << (Anarchy.ID_BITS - (int) distance));
    int shift_by = (Anarchy.ID_BITS - (int) distance);
    return (
      (x << (int) distance)
    | (fall_off >> shift_by)
    );
  }

  public ulong fold(ulong x, ulong where) {
    // Folds lower bits into upper bits using xor. 'where' is restricted to
    // fall between 1/4 and 1/2 of ID_BITS.
    int quarter = Anarchy.ID_BITS / 4;
    where = posmod(where, quarter) + (ulong) quarter;
    ulong m = mask((int) where);
    ulong lower = x & m;
    int shift_by = Anarchy.ID_BITS - (int) where;
    return x ^ (lower << shift_by);
  }
  // fold is its own inverse.

  public static ulong FLOP_MASK = 0xf0f0f0f0f0f0f0f0;

  public ulong flop(ulong x) {
    // Flops each 1/2 byte with the adjacent 1/2 byte.
    ulong left = x & Anarchy.FLOP_MASK;
    ulong right = x & ~Anarchy.FLOP_MASK;
    return (right << 4) | (left >> 4);
  }
  // flop is its own inverse.

  public ulong scramble(ulong x) {
    // Implements a reversible linear-feedback-shift-register-like operation.
    // TODO: 64-bit version?
    ulong trigger = x & 0x80200003;
    ulong r = swirl(x, 1);
    r ^= 0x03040610 * trigger; // pseudo-if
    return r;
  }

  public ulong rev_scramble(ulong x) {
    // Inverse of scramble (see above).
    ulong pr = rev_swirl(x, 1);
    ulong trigger = pr & 0x80200003;
    pr ^= 0x06080c20 * trigger; // pseudo-if; this is rev_swirl(0x03040610, 1)
    return pr;
  }

  public ulong scramble_seed(ulong s) {
    // Scrambles a seed value to help separate RNG sequences generated from
    // sequential seeds.

    s = (s + 1) * (3 + posmod(s, 23));
    s = fold(s, 11); // prime
    s = scramble(s);
    s = swirl(s, s + 23); // prime
    s = scramble(s);
    s ^= posmod(s, 153) * scramble(s);

    return s;
  }

  public ulong prng(ulong x, ulong seed) {
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
    return x;
  }

  public ulong rev_prng(ulong x, ulong seed) {
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
    return x;
  }

  public ulong lfsr(ulong x) {
    // Implements a max-cycle-length 64-bit linear-feedback-shift-register.
    // See: https://en.wikipedia.org/wiki/Linear-feedback_shift_register
    // Note that this is NOT reversible!
    //
    // See also: http://courses.cse.tamu.edu/walker/csce680/lfsr_table.pdf
    ulong lsb = x & 1;
    ulong r = x >> 1;
    // pseudo-if
    r ^= lsb * 0xe800000000000000; // 64, 63, 61, 60
    return r;
  }

  public double udist(ulong seed) {
    // Generates a random number between 0 and 1 given a seed value.
    ulong ux = lfsr(seed);
    ulong sc = ux ^ (ux << 16);
    return posmod(sc, 2147483659) / 2147483659.0; // prime near 2^31
  }

  public double pgdist(ulong seed) {
    // Generates and averages three rando numbers between 0 and 1 to give a
    // pseudo-gaussian-distributed random number (still strictly on [0, 1) )
    double t = 0;
    for (ulong i = 0; i < 3; i += 1) {
      t += udist(seed + 9182793183*i);
    }
    return t/3.0;
  }

  public bool flip(double p, ulong seed) {
    // Flips a coin with probability p of being True. Using the same seed
    // always give the same result.
    return udist(seed) < p;
  }

  public ulong idist(ulong seed, ulong start, ulong end) {
    // Even distribution over the given integer range, including the lower end
    // but excluding the higher end (even if the lower end is given second).
    // Distribution bias is about one part in (range/2^31).
    return (ulong) Math.Floor(udist(seed) * (end - start)) + start;
  }

  public double expdist(ulong seed) {
    // Generates a number from an exponential distribution with mean 0.5 given
    // a seed. See:
    // https://math.stackexchange.com/questions/28004/random-exponential-like-distribution
    double u = udist(seed);
    return -Math.Log(1 - u)/0.5;
  }

  public ulong cohort(ulong outer, ulong cohort_size) {
    // Computes cohort number for the given outer index and cohort size.
    return outer / cohort_size;
  }

  public ulong cohort_inner(ulong outer, ulong cohort_size) {
    // Computes within-cohort index for the given outer index and cohorts of
    // the given size.
    return posmod(outer, cohort_size);
  }

  /* TODO
  function cohort_and_inner(outer, cohort_size) {
    // Returns an array containing both the cohort number and inner index for
    // the given outer index and cohort size.
    return [ cohort(outer, cohort_size), cohort_inner(outer, cohort_size) ];
  }
  */

  public ulong cohort_outer(ulong cohort, ulong inner, ulong cohort_size) {
    // Inverse of cohort_and_inner; computes the outer index from a cohort
    // number and inner index.
    return cohort_size * cohort + inner;
  }

  public ulong cohort_interleave(ulong inner, ulong cohort_size) {
    // Interleaves cohort members by folding the top half into the bottom half.
    if (inner < (cohort_size+1)/2) {
      return inner * 2;
    } else {
      return ((cohort_size - 1 - inner) * 2) + 1;
    }
  }

  public ulong rev_cohort_interleave(ulong inner, ulong cohort_size) {
    // Inverse interleave (see above).
    if (posmod(inner, 2) != 0) {
      return cohort_size - 1 - inner/2;
    } else {
      return inner/2;
    }
  }

  public ulong cohort_fold(ulong inner, ulong cohort_size, ulong seed) {
    // Folds items past an arbitrary split point (in the second half of the
    // cohort) into the middle of the cohort. The split will always leave an
    // odd number at the end.
    ulong half = cohort_size / 2;
    ulong quarter = cohort_size / 4;
    ulong split = half;
    if (quarter > 0) {
      split += posmod(seed, quarter);
    }
    ulong after = cohort_size - split;
    split += posmod(after + 1, 2); // force an odd split point
    after = cohort_size - split;

    ulong fold_to = half - after / 2;

    if (inner < fold_to) { // first region
      return inner;
    } else if (inner < split) { // second region
      return inner + after; // push out past fold region
    } else { // fold region
      return inner - split + fold_to;
    }
  }

  public ulong rev_cohort_fold(ulong inner, ulong cohort_size, ulong seed) {
    // Inverse fold (see above).
    ulong half = cohort_size / 2;
    ulong quarter = cohort_size / 4;
    ulong split = half;
    if (quarter > 0) {
      split += posmod(seed, quarter);
    }
    ulong after = cohort_size - split;
    split += posmod((after + 1), 2); // force an odd split point
    after = cohort_size - split;

    ulong fold_to = half - after / 2;

    if (inner < fold_to) { // first region
      return inner;
    } else if (inner < fold_to + after) { // second region
      return inner - fold_to + split;
    } else {
      return inner - after;
    }
  }

  public ulong cohort_spin(ulong inner, ulong cohort_size, ulong seed) {
    // Applies a circular offset
    return posmod((inner + seed), cohort_size) >> 0;
  }

  public ulong rev_cohort_spin(ulong inner, ulong cohort_size, ulong seed) {
    // Inverse spin (see above).
    return posmod(
      inner + (cohort_size - posmod(seed, cohort_size)),
      cohort_size
    );
  }

  public ulong cohort_flop(ulong inner, ulong cohort_size, ulong seed) {
    // Flops sections (with seeded sizes) with their neighbors.
    ulong limit = cohort_size / 8;
    if (limit < 4) {
      limit += 4;
    }
    ulong size = posmod(seed, limit) + 2;
    ulong which = inner / size;
    ulong local = posmod(inner, size);

    ulong result = 0;
    if (posmod(which, 2) > 0) {
      result = (which - 1) * size + local;
    } else {
      result = (which + 1) * size + local;
    }

    if (result >= cohort_size) { // don't flop out of the cohort
      return inner;
    } else {
      return result;
    }
  }
  // flop is its own inverse

  public ulong cohort_mix(ulong inner, ulong cohort_size, ulong seed) {
    // Applies a spin to both even and odd items with different seeds.
    ulong even = inner - posmod(inner, 2);
    ulong target = 0;
    if (posmod(inner, 2) > 0) {
      target = cohort_spin(
        even / 2,
        cohort_size / 2,
        seed + 464185
      );
      return 2 * target + 1;
    } else {
      target = cohort_spin(
        even / 2,
        (cohort_size + 1) / 2,
        seed + 1048239
      );
      return 2 * target;
    }
  }

  public ulong rev_cohort_mix(ulong inner, ulong cohort_size, ulong seed) {
    // Inverse mix (see above).
    ulong even = inner - posmod(inner, 2);
    ulong target = 0;
    if (posmod(inner, 2) > 0) {
      target = rev_cohort_spin(
        even / 2,
        cohort_size / 2,
        seed + 464185
      );
      return 2 * target + 1;
    } else {
      target = rev_cohort_spin(
        even / 2,
        (cohort_size + 1) / 2,
        seed + 1048239
      );
      return 2 * target;
    }
  }

  public ulong MIN_REGION_SIZE = 2;
  public ulong MAX_REGION_COUNT = 16;

  public ulong cohort_spread(ulong inner, ulong cohort_size, ulong seed) {
    // Spreads items out between a random number of different regions within
    // the cohort.
    ulong min_regions = 2;
    if (cohort_size < 2 * MIN_REGION_SIZE) {
      min_regions = 1;
    }
    ulong max_regions = 1 + cohort_size / MIN_REGION_SIZE;
    ulong regions = (
      min_regions + posmod(
        posmod(seed, (1 + (max_regions - min_regions))),
        MAX_REGION_COUNT
      )
    );
    ulong region_size = cohort_size / regions;
    ulong leftovers = cohort_size - (regions * region_size);

    ulong region = posmod(inner, regions);
    ulong index = inner / regions;
    if (index < region_size) { // non-leftovers
      return region * region_size + index + leftovers;
    } else { // leftovers go at the front:
      return inner - regions * region_size;
    }
  }

  public ulong rev_cohort_spread(ulong inner, ulong cohort_size, ulong seed) {
    // Inverse spread (see above).
    ulong min_regions = 2;
    if (cohort_size < 2 * MIN_REGION_SIZE) {
      min_regions = 1;
    }
    ulong max_regions = 1 + cohort_size / MIN_REGION_SIZE;
    ulong regions = (
      min_regions + posmod(
        posmod(seed, (1 + (max_regions - min_regions))),
        MAX_REGION_COUNT
      )
    );

    ulong region_size = cohort_size / regions;
    ulong leftovers = cohort_size - (regions * region_size);

    ulong index = (inner - leftovers) / region_size;
    ulong region = posmod((inner - leftovers), region_size);

    if (inner < leftovers) { // leftovers back to the end:
      return regions * region_size + inner;
    } else {
      return region * regions + index;
    }
  }

  public ulong cohort_upend(ulong inner, ulong cohort_size, ulong seed) {
    // Reverses ordering within each of several fragments.
    ulong min_regions = 2;
    if (cohort_size < 2 * MIN_REGION_SIZE) {
      min_regions = 1;
    }
    ulong max_regions = 1 + cohort_size / MIN_REGION_SIZE;
    ulong regions = (
      min_regions + posmod(
        posmod(seed, (1 + (max_regions - min_regions))),
        MAX_REGION_COUNT
      )
    );
    ulong region_size = cohort_size / regions;

    ulong region = inner / region_size;
    ulong index = posmod(inner, region_size);

    ulong result = (region * region_size) + (region_size - 1 - index);

    if (result < cohort_size) {
      return result;
    } else {
      return inner;
    }
  }
  // Upend is its own inverse.

  public ulong cohort_shuffle(ulong inner, ulong cohort_size, ulong seed) {
    // Compose a bunch of the above functions to perform a nice thorough
    // shuffle within a cohort.
    ulong r = inner;
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

  public ulong rev_cohort_shuffle(ulong inner, ulong cohort_size, ulong seed) {
    // Inverse shuffle (see above).
    ulong r = inner;
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
}
