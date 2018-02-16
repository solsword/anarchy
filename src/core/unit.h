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

// Mask containing every-other byte.
#define FLOP_MASK 0xf0f0f0f0f0f0f0f0

// An ID to be used for out-of-band purposes. Note that it's often not strictly
// out-of-band, however.
#define NONE 0

/********************
 * Inline Functions *
 ********************/

static inline id acy_mask(id bits) {
  return (1ULL << bits) - 1ULL;
}

static inline id acy_byte_mask(id byte) {
  return (0xffULL) << (byte * 8ULL);
}

static inline id acy_min(id A, id B) {
  return (A < B ? A : B);
}

static inline id acy_max(id A, id B) {
  return (A > B ? A : B);
}

// A circular bit shift; distance is capped at 3/4 of ID_BITS
static inline id acy_circular_shift(id x, id distance) {
  distance %= ((ID_BITS << 1) + ID_BITS) >> 2; // 3/4 of ID_BITS
  id m = acy_mask(distance);
  id fall_off = x & m;
  id shift_by = (ID_BITS - distance);
  return (
    (x >> distance)
  | (fall_off << shift_by)
  );
}

// Reverse
static inline id acy_rev_circular_shift(id x, id distance) {
  distance %= ((ID_BITS << 1) + ID_BITS) >> 2; // 3/4 of ID_BITS
  id m = acy_mask(distance);
  id fall_off = x & (m << (ID_BITS - distance));
  id shift_by = (ID_BITS - distance);
  return (
    (x << distance)
  | (fall_off >> shift_by)
  );
}

// Folds lower bits into upper bits using xor. Where is restricted to fall
// between 1/4 and 1/2 of ID_BITS. Note that this function is its own inverse.
static inline id acy_fold(id x, id where) {
  where %= (ID_BITS >> 2);
  where += (ID_BITS >> 2);
  id m = acy_mask(where);
  id lower = x & m;
  id shift_by = ID_BITS - where;
  return x ^ (lower << shift_by);
}

// Flops each half-byte with the adjacent half-byte. This is its own inverse.
static inline id acy_flop(id x) {
  id left = x & FLOP_MASK;
  id right = x & ~FLOP_MASK;
  return (right << 4ULL) | (left >> 4ULL);
}

// Implements something similar to a linear-feedback-shift-register, but
// reversible. Note that the shift does not cause the trigger mask to overlap
// the scramble mask, which would otherwise prevent reversibility.
static inline id acy_scramble(id x) {
  id trigger = !!(x & 0x80200003); // 1 or 0
  x = acy_circular_shift(x, 1);
  x ^= trigger * 0x03040610; // pseudo-if
  return x;
}

static inline id acy_rev_scramble(id x) {
  x = acy_rev_circular_shift(x, 1);
  id trigger = !!(x & 0x80200003); // 1 or 0
  x ^= trigger * 0x06080c20; // pseudo-if; constant above left-shifted by 1
  return x;
}

// A simple reversible pseudo-random number generator
static inline id acy_prng(id x, id seed) {
  x += 13; // prime
  x = acy_fold(x, seed + 17); // prime
  x = acy_flop(x);
  x = acy_circular_shift(x, seed + 37); // prime
  x = acy_fold(x, seed + 89); // prime
  x = acy_circular_shift(x, seed + 107); // prime
  x = acy_flop(x);
  return x;
}

// Reverse
static inline id acy_rev_prng(id x, id seed) {
  x = acy_flop(x);
  x = acy_rev_circular_shift(x, seed + 107); // prime
  x = acy_fold(x, seed + 89); // prime
  x = acy_rev_circular_shift(x, seed + 37); // prime
  x = acy_flop(x);
  x = acy_fold(x, seed + 17); // prime
  x -= 13; // prime
  return x;
}

// A smoothed prng with an integer limit. This is non-reversible for several
// reasons, including the modulus to fit within the limit and the averaging. If
// smoothness is set to zero, this is the same as just prng with a modulus.
// Note that if the modulus is too large, integer overflow during addition
// before averaging will destroy the smooth property of results.
static inline id acy_irrev_smooth_prng(id x, id limit, id smoothness, id seed) {
  id random = acy_prng(x, seed);
  id result = random % limit;
  for (id i = 0; i < smoothness; ++i) {
    random = acy_prng(random, seed);
    result += random % limit;
  }
  result /= (smoothness + 1);
  return result;
}

#endif // INCLUDE_UNIT_H
