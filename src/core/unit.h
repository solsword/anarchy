/**
 * @file: unit.h
 *
 * @description: Reversible unit operations; definition of the "id" type.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#ifndef INCLUDE_UNIT_H
#define INCLUDE_UNIT_H

#include <stdint.h>

/************************
 * Types and Structures *
 ************************/

// An ID is 64 bits, unsigned so that shifts don't do sign extension
#define ID_BITS 64ULL
#define ID_BYTES 8ULL
typedef uint64_t id;

// An ID to be used for out-of-band purposes. Note that it's often not strictly
// out-of-band, however.
#define NONE 0

/********************
 * Inline Functions *
 ********************/

static inline id myc_mask(id bits) {
  return (1ULL << bits) - 1ULL;
}

static inline id myc_byte_mask(id byte) {
  return (0xffULL) << (byte * 8ULL);
}

static inline id myc_min(id A, id B) {
  return (A < B ? A : B);
}

static inline id myc_max(id A, id B) {
  return (A > B ? A : B);
}

// A circular bit shift; distance is capped at 3/4 of ID_BITS
static inline id myc_circular_shift(id x, id distance) {
  distance %= ((ID_BITS << 1) + ID_BITS) >> 2; // 3/4 of ID_BITS
  id m = myc_mask(distance);
  id fall_off = x & m;
  id shift_by = (ID_BITS - distance);
  return (
    (x >> distance)
  | (fall_off << shift_by)
  );
}

// Reverse
static inline id myc_rev_circular_shift(id x, id distance) {
  distance %= ((ID_BITS << 1) + ID_BITS) >> 2; // 3/4 of ID_BITS
  id m = myc_mask(distance);
  id fall_off = x & (m << (ID_BITS - distance));
  id shift_by = (ID_BITS - distance);
  return (
    (x << distance)
  | (fall_off >> shift_by)
  );
}

// Folds lower bits into upper bits using xor. Where is restricted to fall
// between 1/4 and 1/2 of ID_BITS. Note that this function is its own inverse.
static inline id myc_fold(id x, id where) {
  where %= (ID_BITS >> 2);
  where += (ID_BITS >> 2);
  id m = myc_mask(where);
  id lower = x & m;
  id shift_by = ID_BITS - where;
  return x ^ (lower << shift_by);
}

// Flops each byte with the adjacent byte. This is its own inverse.
static inline id myc_flop(id x) {
  for (int i = 0; i < ID_BYTES - 1; i += 2) {
    id m = myc_byte_mask(i);
    id om = myc_byte_mask(i+1);
    id fb = x & m;
    id sb = x & om;
    x &= ~m;
    x &= ~om;
    x |= fb << 8ULL;
    x |= sb >> 8ULL;
  }
  return x;
}

// A simple reversible pseudo-random number generator
static inline id myc_prng(id x, id seed) {
  x ^= seed;
  x = myc_flop(x);
  x = myc_fold(x, seed + 17);
  x = myc_circular_shift(x, seed + 48);
  x = myc_fold(x, seed + 83);
  x = myc_circular_shift(x, seed + 105);
  x = myc_flop(x);
  return x;
}

// Reverse
static inline id myc_rev_prng(id x, id seed) {
  x = myc_flop(x);
  x = myc_rev_circular_shift(x, seed + 105);
  x = myc_fold(x, seed + 83);
  x = myc_rev_circular_shift(x, seed + 48);
  x = myc_fold(x, seed + 17);
  x = myc_flop(x);
  x ^= seed;
  return x;
}

// A smoothed prng with an integer limit. This is non-reversible for several
// reasons, including the modulus to fit within the limit and the averaging. If
// smoothness is set to zero, this is the same as just prng with a modulus.
// Note that if the modulus is too large, integer overflow during addition
// before averaging will destroy the smooth property of results.
static inline id myc_irrev_smooth_prng(id x, id limit, id smoothness, id seed) {
  id random = myc_prng(x, seed);
  id result = random % limit;
  for (id i = 0; i < smoothness; ++i) {
    random = myc_prng(random, seed);
    result += random % limit;
  }
  result /= (smoothness + 1);
  return result;
}

#endif // INCLUDE_UNIT_H
