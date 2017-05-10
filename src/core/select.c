/**
 * @file: select.c
 *
 * @description: Reversible selection for one-to-many and many-to-one
 * relationships.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <assert.h> // for assert
#include <math.h> // for log2

#include "core/cohort.h" // for cohort operations

#include "select.h"

/*************
 * Functions *
 *************/

void myc_select_parent_and_index(
  id child,
  id avg_arity,
  id max_arity,
  id seed,
  id *r_parent,
  id *r_index
) {
  // NONE is its own parent and is the NONEth child of that parent:
  if (child == NONE) {
    *r_parent = NONE;
    *r_index = NONE;
    return;
  }

  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so

  // Get from absolute-child to child-within-cohort. Note that children in xth
  // child cohort have parents in the xth parent cohort.
  id cohort, inner;
  myc_cohort_and_inner(child, max_arity, seed, &cohort, &inner);

  // Shuffle child ID within children cohort:
  id shuf = myc_cohort_shuffle(inner, max_arity, seed);

  id from_upper = 0;
  id to_upper = upper_cohort_size;
  id parents_left = upper_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = max_arity;
  id children_left = max_arity;

  id divide_at = cohort + seed;

  while (parents_left > 1) {

    half_remaining = parents_left/2;
    divide_at = myc_irrev_smooth_prng(
      divide_at,
      children_left,
      myc_min(2, parents_left),
      seed
    );

    if (shuf >= divide_at) {
      shuf -= divide_at;
      from_lower += divide_at;
      from_upper += half_remaining;
    } else {
      to_lower -= (children_left - divide_at);
      to_upper -= (parents_left - half_remaining);
    }
    parents_left = to_upper - from_upper;
    children_left = to_lower - from_lower;
  }

  // At this point, we know the child's index within its parent's children:
  *r_index = shuf;

  // Unshuffle the parent's index (from_upper)
  id unshuf = myc_rev_cohort_shuffle(from_upper, upper_cohort_size, seed);

  // Escape the cohort to get the parent:
  *r_parent = myc_cohort_outer(cohort, unshuf, upper_cohort_size, seed);
}

id myc_select_nth_child(
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

  myc_cohort_and_inner(parent, upper_cohort_size, seed, &cohort, &inner);

  id shuf = myc_cohort_shuffle(inner, upper_cohort_size, seed);

  id from_upper = 0;
  id to_upper = upper_cohort_size;
  id parents_left = upper_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = max_arity;
  id children_left = max_arity;

  id divide_at = cohort + seed;

  while (parents_left > 1 && children_left > 0) {

    half_remaining = parents_left/2;
    divide_at = myc_irrev_smooth_prng(
      divide_at,
      children_left,
      myc_min(2, parents_left),
      seed
    );

    if (shuf >= half_remaining) {
      shuf -= half_remaining;
      from_lower += divide_at;
      from_upper += half_remaining;
    } else {
      to_lower -= (children_left - divide_at);
      to_upper -= (parents_left - half_remaining);
    }
    parents_left = to_upper - from_upper;
    children_left = to_lower - from_lower;
  }

  if (nth >= children_left) {
    return NONE;
  }

  // Unshuffle child ID within children cohort:
  id unshuf = myc_rev_cohort_shuffle(from_lower + nth, max_arity, seed);

  // Get back from child-within-cohort to absolute-child. Note that children of
  // parents in the xth parent cohort are assigned to the xth child cohort.
  return myc_cohort_outer(cohort, unshuf, max_arity, seed);
}

void myc_select_exp_parent_and_index(
  id child,
  id avg_arity,
  id max_arity,
  float exp_cohort_shape,
  id exp_cohort_size,
  id seed,
  id *r_parent,
  id *r_index
) {
  // NONE is its own parent and is the NONEth child of that parent:
  if (child == NONE) {
    *r_parent = NONE;
    *r_index = NONE;
    return;
  }

  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so

  // Get from absolute-child to child-within-cohort. For exponential child
  // super-cohorts, parents in the xth cohort have children drawn from the
  // x%Nth sub-cohort of the x/Nth exponential super-cohort, where N is
  // exp_cohort_size.
  id super_cohort, sub_cohort, inner;
  // exponential super-cohort 
  myc_exp_cohort_and_inner(
    child,
    exp_cohort_shape,
    max_arity * exp_cohort_size,
    seed,
    &super_cohort,
    &inner
  );

  // shuffle within the exponential cohort:
  inner = myc_cohort_shuffle(inner, max_arity * exp_cohort_size, seed);

  // Find sub-cohort:
  myc_cohort_and_inner(inner, max_arity, 0, &sub_cohort, &inner);

  // Compute parent cohort:
  id parent_cohort = super_cohort * exp_cohort_size + sub_cohort;

  // Shuffle child ID within children cohort:
  id shuf = myc_cohort_shuffle(inner, max_arity, seed);

  id from_upper = 0;
  id to_upper = upper_cohort_size;
  id parents_left = upper_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = max_arity;
  id children_left = max_arity;

  id divide_at = parent_cohort + seed;

  while (parents_left > 1) {
    half_remaining = parents_left/2;
    divide_at = myc_irrev_smooth_prng(
      divide_at,
      children_left,
      myc_min(2, parents_left),
      seed
    );

    if (shuf >= divide_at) {
      shuf -= divide_at;
      from_lower += divide_at;
      from_upper += half_remaining;
    } else {
      to_lower -= (children_left - divide_at);
      to_upper -= (parents_left - half_remaining);
    }
    parents_left = to_upper - from_upper;
    children_left = to_lower - from_lower;
  }

  // At this point, we know the child's index within its parent's children:
  *r_index = shuf;

  // Unshuffle the parent's index (from_upper)
  id unshuf = myc_rev_cohort_shuffle(from_upper, upper_cohort_size, seed);

  // Escape the cohort to get the parent:
  *r_parent = myc_cohort_outer(parent_cohort, unshuf, upper_cohort_size, seed);
}

id myc_select_exp_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  float exp_cohort_shape,
  id exp_cohort_size,
  id seed
) {
  // TODO: HERE!
  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id parent_cohort;
  id inner;
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so
  myc_cohort_and_inner(parent, upper_cohort_size, seed, &parent_cohort, &inner);
  id shuf = myc_cohort_shuffle(inner, upper_cohort_size, seed);

  id from_upper = 0;
  id to_upper = upper_cohort_size;
  id parents_left = upper_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = max_arity;
  id children_left = max_arity;

  id divide_at = parent_cohort + seed;

  while (parents_left > 1 && children_left > 0) {
    half_remaining = parents_left/2;
    divide_at = myc_irrev_smooth_prng(
      divide_at,
      children_left,
      myc_min(2, parents_left),
      seed
    );

    if (shuf >= half_remaining) {
      shuf -= half_remaining;
      from_lower += divide_at;
      from_upper += half_remaining;
    } else {
      to_lower -= (children_left - divide_at);
      to_upper -= (parents_left - half_remaining);
    }
    parents_left = to_upper - from_upper;
    children_left = to_lower - from_lower;
  }

  if (nth >= children_left) {
    return NONE;
  }

  // Unshuffle child ID within children cohort:
  id unshuf = myc_rev_cohort_shuffle(from_lower + nth, max_arity, seed);

  // Get back from child-within-sub-cohort-within-super-cohort to
  // absolute-child. Note that children of parents in the xth parent cohort are
  // assigned to the xth super cohort, and they are also placed into the
  // x+seedth sub-cohort within that super-cohort.
  id outer = myc_cohort_outer(
    parent_cohort % exp_cohort_size,
    unshuf,
    max_arity,
    0
  );

  // Unshuffle within exponential cohort
  unshuf = myc_rev_cohort_shuffle(outer, max_arity * exp_cohort_size, seed);

  // Escape from exponential cohort
  return myc_exp_cohort_outer(
    parent_cohort / exp_cohort_size,
    unshuf,
    exp_cohort_shape,
    max_arity * exp_cohort_size,
    seed
  );
}
