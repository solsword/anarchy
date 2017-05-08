/**
 * @file: select.h
 *
 * @description: Reversible selection for one-to-many and many-to-one
 * relationships.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#ifndef INCLUDE_SELECT_H
#define INCLUDE_SELECT_H

#include <math.h> // for log2
#include <stdlib.h> // for min

#include "core/unit.h" // for "id" and unit operations

// Returns the number of children that the given parent has. This can be
// anywhere from 0 to the given maximum arity.
static inline id select_child_arity(
  id parent,
  id avg_arity,
  id max_arity,
  id seed
) {
  id 
}

// Identifies a parent, as well as which child of that parent the child is.
// Returns these values via the r_parent and r_index parameters.
static inline void select_parent_and_index(
  id child,
  id avg_arity,
  id max_arity,
  id seed,
  id *r_parent,
  id *r_index
) {
  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so

  id cohort, inner;
  cohort_and_inner(parent, upper_cohort_size, seed, &cohort, &inner);
  id shuf = cohort_shuffle(inner, upper_cohort_size, seed);

  id from_upper = 0;
  id to_upper = upper_cohort_size;
  id parents_left = upper_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = max_arity;
  id children_left = max_arity;

  id divide_at = shuf;

  while (parents_left > 1 && children_left > 0) {
    half_remaining = parents_left/2;
    divide_at = irrev_smooth_prng(
      divide_at,
      children_left,
      min(2, parents_left),
      seed
    );

    if (shuf > half_remaining) {
      shuf -= half_remaining;
      children_left = -= divide_at;
      from_lower = divide_at;
      from_upper = half_remaining;
    } else {
      to_lower = divide_at;
      to_upper = half_remaining;
    }
    parents_left = half_remaining;
    children_left = to_lower - to_upper;
  }

  if (nth >= children_left) {
    return NONE;
  }

  //------//

  // NONE is its own parent and is the NONEth child of that parent:
  if (child == NONE) {
    *r_parent = NONE;
    *r_cohort = NONE;
    return;
  }

  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so

  // TODO: HERE!

  // Get from absolute-child to child-within-cohort. Note that children in xth
  // child cohort have parents in the xth parent cohort.
  id cohort, inner;
  cohort_and_inner(child, max_arity, seed, &cohort, &inner);

  // Shuffle child ID within children cohort:
  id shuf = cohort_shuffle(inner, max_arity, seed);
}

// Given a parent and an index, returns the id of that child. If the index is
// out-of-bounds, it returns NONE. Note that the given max arity establishes a
// cohort of children to be divided between a number of parents such that on
// average each parent with have avg_arity children, but integer divisions mean
// that avg_arity is only roughly respected.
static inline id select_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  id seed
) {
  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id cohort;
  id inner;
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so
  cohort_and_inner(parent, upper_cohort_size, seed, &cohort, &inner);
  id shuf = cohort_shuffle(inner, upper_cohort_size, seed);

  id from_upper = 0;
  id to_upper = upper_cohort_size;
  id parents_left = upper_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = max_arity;
  id children_left = max_arity;

  id divide_at = shuf;

  while (parents_left > 1 && children_left > 0) {
    half_remaining = parents_left/2;
    divide_at = irrev_smooth_prng(
      divide_at,
      children_left,
      min(2, parents_left),
      seed
    );

    if (shuf > half_remaining) {
      shuf -= half_remaining;
      children_left = -= divide_at;
      from_lower = divide_at;
      from_upper = half_remaining;
    } else {
      to_lower = divide_at;
      to_upper = half_remaining;
    }
    parents_left = half_remaining;
    children_left = to_lower - to_upper;
  }

  if (nth >= children_left) {
    return NONE;
  }
  // Unshuffle child ID within children cohort:
  id unshuf = rev_cohort_shuffle(from_lower + nth, max_arity, seed);
  // Get back from child-within-cohort to absolute-child. Note that children of
  // parents in the xth parent cohort are assigned to the xth child cohort.
  return cohort_outer(cohort, unshuf, max_arity, seed);
}

#endif // INCLUDE_SELECT_H
