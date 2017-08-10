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
#ifdef DEBUG_SELECT
  #include <stdio.h>
#endif

#include "core/cohort.h" // for cohort operations

#include "select.h"

/*************
 * Functions *
 *************/

void acy_select_parent_and_index(
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

  // Un-correct child indices since they're >= parent indices:
  child -= max_arity;

  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so

  // Get from absolute-child to child-within-cohort. Note that children in xth
  // child cohort have parents in the xth parent cohort.
  id cohort, inner;
  acy_mixed_cohort_and_inner(child, max_arity, seed, &cohort, &inner);

  // Shuffle child ID within children cohort:
  id shuf = acy_cohort_shuffle(inner, max_arity, seed);

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
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
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
  id unshuf = acy_rev_cohort_shuffle(from_upper, upper_cohort_size, seed);

  // Escape the cohort to get the parent:
  *r_parent = acy_mixed_cohort_outer(cohort, unshuf, upper_cohort_size, seed);
}

id acy_select_nth_child(
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

  acy_mixed_cohort_and_inner(parent, upper_cohort_size, seed, &cohort, &inner);

  id shuf = acy_cohort_shuffle(inner, upper_cohort_size, seed);

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
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
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
  id unshuf = acy_rev_cohort_shuffle(from_lower + nth, max_arity, seed);

  // Get back from child-within-cohort to absolute-child. Note that children of
  // parents in the xth parent cohort are assigned to the xth child cohort.
  id child = acy_mixed_cohort_outer(cohort, unshuf, max_arity, seed);

  // Correct child indices so that they're >= parent indices:
  return child + max_arity;
}

id acy_count_select_children(
  id parent,
  id avg_arity,
  id max_arity,
  id seed
) {
  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id cohort;
  id inner;
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so

  acy_mixed_cohort_and_inner(parent, upper_cohort_size, seed, &cohort, &inner);

  id shuf = acy_cohort_shuffle(inner, upper_cohort_size, seed);

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
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
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

  return children_left;
}

id acy_select_exp_earliest_possible_child(
  id parent,
  id avg_arity,
  id max_arity,
  id exp_cohort_size,
  id exp_cohort_layers
) {
  // Cohort sizes
  id upper_cohort_size = max_arity / avg_arity;
  id lower_cohort_size = max_arity * exp_cohort_size;
  id mega_cohort_size = max_arity * exp_cohort_size * exp_cohort_layers;

  // Find parent cohort
  id parent_cohort = parent / (upper_cohort_size * exp_cohort_layers);

  // Compute child cohort
  id child_cohort = parent_cohort / exp_cohort_size;

  //id min_child_cohort = child_cohort - exp_cohort_layers;

  return mega_cohort_size * (
    (child_cohort * lower_cohort_size) / mega_cohort_size
  );

  /*

  // Escape at the bottom of the exponential child cohort
  return (
    lower_cohort_size
  * min_child_cohort
  / exp_cohort_layers
  );
  */
}

id acy_select_exp_child_cohort_start(
  id child,
  id avg_arity,
  id max_arity,
  id exp_cohort_size,
  id exp_cohort_layers
) {
  // Cohort sizes
  // id upper_cohort_size = max_arity / avg_arity;
  id lower_cohort_size = max_arity * exp_cohort_size;
  id mega_cohort_size = max_arity * exp_cohort_size * exp_cohort_layers;

  return mega_cohort_size * (child / mega_cohort_size);

  /*
   * TODO: Get rid of this?
  // Compute child cohort
  id child_cohort = child / lower_cohort_size;

  // Escape at the bottom of the exponential child cohort
  return lower_cohort_size * child_cohort;
  */
}

void acy_select_exp_parent_and_index(
  id child,
  id avg_arity,
  id max_arity,
  double exp_cohort_shape,
  id exp_cohort_size,
  id exp_cohort_layers,
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

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "\nselect_exp_parent_and_index::child/avg/max::%lu/%lu/%lu\n",
    child, avg_arity, max_arity
  );
  fprintf(
    stderr,
    "select_exp_parent_and_index::shape/size/layers::%.3f/%lu/%lu\n",
    exp_cohort_shape, exp_cohort_size, exp_cohort_layers
  );
#endif

  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so
  id lower_cohort_size = max_arity * exp_cohort_size;

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_parent_and_index::ucs/lcs::%lu/%lu\n",
    upper_cohort_size,
    lower_cohort_size
  );
#endif

  /*
  id adjusted = child - acy_select_exp_child_cohort_start(
    child,
    avg_arity,
    max_arity,
    exp_cohort_size,
    exp_cohort_layers
  );

  if (adjusted > child) { // underflow
    *r_parent = NONE;
    *r_index = NONE;
    return;
  }
  */
  // TODO: DEBUG
  id adjusted = child;

  // Get from absolute-child to child-within-cohort. For exponential child
  // super-cohorts, parents in the xth cohort have children drawn from the
  // x%Nth sub-cohort of the x/Nth exponential super-cohort, where N is
  // exp_cohort_size.
  id super_cohort, sub_cohort, inner;
  // exponential super-cohort 
  acy_multiexp_cohort_and_inner(
    adjusted,
    exp_cohort_shape,
    lower_cohort_size,
    exp_cohort_layers,
    seed,
    &super_cohort,
    &inner
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_parent_and_index::super/inner::%lu/%lu\n",
    super_cohort,
    inner
  );
#endif

  // shuffle within the exponential cohort:
  inner = acy_cohort_shuffle(inner, lower_cohort_size, seed);

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_exp_parent_and_index::shuffled_inner::%lu\n", inner);
#endif

  // Find sub-cohort:
  acy_cohort_and_inner(inner, max_arity, &sub_cohort, &inner);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_parent_and_index::sub/inner::%lu/%lu\n",
    sub_cohort,
    inner
  );
#endif

  // Compute parent cohort:
  id parent_cohort = super_cohort * exp_cohort_size + sub_cohort;

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_parent_and_index::child/parent_cohort::%lu/%lu\n",
    child / upper_cohort_size,
    parent_cohort
  );
#endif

  // Shuffle child ID within children cohort:
  id shuf = acy_cohort_shuffle(inner, max_arity, seed);

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_exp_parent_and_index::child_shuf::%lu\n", shuf);
#endif

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
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
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

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_exp_parent_and_index::divided_shuf::%lu\n", shuf);
#endif

  // At this point, we know the child's index within its parent's children:
  *r_index = shuf;

  // Unshuffle the parent's index (from_upper)
  id unshuf = acy_rev_cohort_shuffle(from_upper, upper_cohort_size, seed);

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_exp_parent_and_index::parent_unshuf::%lu\n", unshuf);
#endif

  // Escape the cohort to get the parent:
  *r_parent = acy_cohort_outer(parent_cohort, unshuf, upper_cohort_size);

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_exp_parent_and_index::parent::%lu\n\n", *r_parent);
#endif
}

id acy_select_exp_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  double exp_cohort_shape,
  id exp_cohort_size,
  id exp_cohort_layers,
  id seed
) {
  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id parent_cohort;
  id inner;
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so
  id lower_cohort_size = max_arity * exp_cohort_size;

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "\nselect_exp_nth_child::parent/nth/avg/max::%lu/%lu/%lu/%lu\n",
    parent, nth, avg_arity, max_arity
  );
  fprintf(
    stderr,
    "select_exp_nth_child::shape/size/layers::%.3f/%lu/%lu\n",
    exp_cohort_shape, exp_cohort_size, exp_cohort_layers
  );
#endif

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::ucs/lcs::%lu/%lu\n",
    upper_cohort_size,
    lower_cohort_size
  );
#endif

  acy_cohort_and_inner(parent, upper_cohort_size, &parent_cohort, &inner);
  id shuf = acy_cohort_shuffle(inner, upper_cohort_size, seed);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::upper_cohort/inner/shuf::%lu/%lu/%lu\n",
    parent_cohort,
    inner,
    shuf
  );
#endif

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
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
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

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::divided_shuf/from_lower::%lu/%lu\n",
    shuf,
    from_lower
  );
#endif

  // Unshuffle child ID within children cohort:
  id unshuf = acy_rev_cohort_shuffle(from_lower + nth, max_arity, seed);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::unshuffled_lower::%lu\n",
    unshuf
  );
#endif

  // Get back from child-within-sub-cohort-within-super-cohort to
  // absolute-child. Note that children of parents in the xth parent cohort are
  // assigned to the x/Nth super cohort and the x%Nth sub cohort, where N is
  // exp_cohort_size.
  id outer = acy_cohort_outer(
    parent_cohort % exp_cohort_size,
    unshuf,
    max_arity
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::subcohort/outer::%lu/%lu\n",
    parent_cohort % exp_cohort_size,
    outer
  );
#endif

  // Unshuffle within exponential cohort
  unshuf = acy_rev_cohort_shuffle(outer, lower_cohort_size, seed);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::unshuf-in-super::%lu\n",
    unshuf
  );
#endif

  // Escape from exponential cohort
  id child = acy_multiexp_cohort_outer(
    parent_cohort / exp_cohort_size,
    unshuf,
    exp_cohort_shape,
    lower_cohort_size,
    exp_cohort_layers,
    seed
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_exp_nth_child::super/result::%lu/%lu\n",
    parent_cohort/exp_cohort_size,
    child
  );
#endif

  /*
  // Adjust index to be >= parent:
  id adjusted = child + parent - acy_select_exp_earliest_possible_child(
    parent,
    avg_arity,
    max_arity,
    exp_cohort_size,
    exp_cohort_layers
  );

  if (adjusted < child) { // overflow
    return NONE;
  }
  */
  // TODO: DEBUG
  id adjusted = child;

  return adjusted;
}

id acy_select_poly_earliest_possible_child(
  id parent,
  id parent_cohort_size,
  id child_cohort_size,
  id poly_cohort_base,
  id poly_cohort_shape,
  id seed
) {
  // Cohort sizes
  id child_super_cohort_size = acy_quadsum(poly_cohort_base, poly_cohort_shape);
  id parent_super_cohort_size = (
    parent_cohort_size
  * (child_super_cohort_size / child_cohort_size)
  );
  id child_leftovers = (
    child_super_cohort_size
  - (child_super_cohort_size / child_cohort_size) * child_cohort_size
  );
  if (child_leftovers > 0) {
    // add a cohort to parent the leftover children
    parent_super_cohort_size += parent_cohort_size;
  }

  // Find parent cohort
  id parent_super_cohort;
  id parent_super_inner;
  acy_cohort_and_inner(
    parent,
    parent_super_cohort_size,
    &parent_super_cohort,
    &parent_super_inner
  );
  //id parent_inner_cohort = acy_cohort(parent_super_inner, parent_cohort_size);

  // Compute child cohort
  id child_super_cohort = parent_super_cohort;

  // Get minimum possible outside value 
  id child_cohort_start = acy_multipoly_cohort_outer(
    child_super_cohort,
    poly_cohort_base - 1,
    poly_cohort_base,
    poly_cohort_shape,
    seed
  );

  return child_cohort_start;
}

void acy_select_poly_parent_and_index(
  id child,
  id parent_cohort_size,
  id child_cohort_size,
  id poly_cohort_base,
  id poly_cohort_shape,
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

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "\nselect_poly_parent_and_index::child/pcs/ccs::%lu/%lu/%lu\n",
    child, parent_cohort_size, child_cohort_size
  );
  fprintf(
    stderr,
    "select_poly_parent_and_index::base/shape::%lu/%lu\n",
    poly_cohort_base, poly_cohort_shape
  );
#endif

  // Get from absolute-child to child-within-cohort. For polynomial child
  // super-cohorts, parents in the xth super-cohort have children drawn from
  // the xth polynomial super-cohort.
  id super_cohort, super_inner;
  // polynomial super-cohort 
  acy_multipoly_cohort_and_inner(
    child,
    poly_cohort_base,
    poly_cohort_shape,
    seed,
    &super_cohort,
    &super_inner
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_parent_and_index::child_super/child_inner::%lu/%lu\n",
    super_cohort,
    super_inner
  );
#endif

  // reverse shuffle within cohort
  id child_super_cohort_size = acy_quadsum(poly_cohort_base, poly_cohort_shape);
  shuf = acy_rev_cohort_shuffle(
    super_inner,
    child_super_cohort_size,
    seed + super_cohort
  );

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_poly_parent_and_index::shuffled_inner::%lu\n", shuf);
#endif

  // now find our sub-cohort
  // if we're in the nth sub-cohort of our super-cohort, our parent is in the
  // nth sub-cohort of their super cohort.
  id sub_cohort, sub_inner;
  acy_cohort_and_inner(
    shuf,
    child_cohort_size,
    &sub_cohort,
    &sub_inner
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_parent_and_index::sub/inner::%lu/%lu\n",
    sub_cohort,
    sub_inner
  );
#endif

  // Shuffle child ID within children cohort:
  id inner_shuf = acy_cohort_shuffle(
    sub_inner,
    child_cohort_size,
    seed + sub_cohort
  );

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_poly_parent_and_index::inner_shuf::%lu\n",inner_shuf);
#endif

  id from_upper = 0;
  id to_upper = parent_cohort_size;
  id parents_left = parent_cohort_size;
  id half_remaining;

  id from_lower = 0;
  id to_lower = child_cohort_size;
  id children_left = child_cohort_size;

  id divide_at = sub_cohort + seed;

  while (parents_left > 1) {
    half_remaining = parents_left/2;
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
      seed
    );

    if (inner_shuf >= divide_at) {
      inner_shuf -= divide_at;
      from_lower += divide_at;
      from_upper += half_remaining;
    } else {
      to_lower -= (children_left - divide_at);
      to_upper -= (parents_left - half_remaining);
    }
    parents_left = to_upper - from_upper;
    children_left = to_lower - from_lower;
  }

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_parent_and_index::divided_shuf::%lu\n",
    inner_shuf
  );
#endif

  // At this point, we know the child's index within its parent's children:
  *r_index = inner_shuf;

  // And the parent's index is from_upper
#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_parent_and_index::index/inner_parent::%lu/%lu\n",
    *r_index,
    from_upper
  );
#endif

  // Escape twice to get the parent:
  id parent_super_inner = acy_cohort_outer(
    sub_cohort,
    from_upper,
    parent_cohort_size
  );

  *r_parent = acy_cohort_outer(
    super_cohort,
    parent_super_inner,
    parent_super_cohort_size
  );

#ifdef DEBUG_SELECT
  fprintf(stderr, "select_poly_parent_and_index::parent::%lu\n\n", *r_parent);
#endif
}

// TODO: HERE
id acy_select_poly_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  double poly_cohort_shape,
  id poly_cohort_size,
  id poly_cohort_layers,
  id seed
) {
  // Otherwise we have just one parent per child cohort
  assert(avg_arity < (max_arity/2));
  id parent_cohort;
  id inner;
  id upper_cohort_size = max_arity / avg_arity; // at least 2, ideally 8+ or so
  id lower_cohort_size = max_arity * poly_cohort_size;

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "\nselect_poly_nth_child::parent/nth/avg/max::%lu/%lu/%lu/%lu\n",
    parent, nth, avg_arity, max_arity
  );
  fprintf(
    stderr,
    "select_poly_nth_child::shape/size/layers::%.3f/%lu/%lu\n",
    poly_cohort_shape, poly_cohort_size, poly_cohort_layers
  );
#endif

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::ucs/lcs::%lu/%lu\n",
    upper_cohort_size,
    lower_cohort_size
  );
#endif

  acy_cohort_and_inner(parent, upper_cohort_size, &parent_cohort, &inner);
  id shuf = acy_cohort_shuffle(inner, upper_cohort_size, seed);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::upper_cohort/inner/shuf::%lu/%lu/%lu\n",
    parent_cohort,
    inner,
    shuf
  );
#endif

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
    divide_at = acy_irrev_smooth_prng(
      divide_at,
      children_left,
      acy_min(2, parents_left),
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

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::divided_shuf/from_lower::%lu/%lu\n",
    shuf,
    from_lower
  );
#endif

  // Unshuffle child ID within children cohort:
  id unshuf = acy_rev_cohort_shuffle(from_lower + nth, max_arity, seed);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::unshuffled_lower::%lu\n",
    unshuf
  );
#endif

  // Get back from child-within-sub-cohort-within-super-cohort to
  // absolute-child. Note that children of parents in the xth parent cohort are
  // assigned to the x/Nth super cohort and the x%Nth sub cohort, where N is
  // poly_cohort_size.
  id outer = acy_cohort_outer(
    parent_cohort % poly_cohort_size,
    unshuf,
    max_arity
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::subcohort/outer::%lu/%lu\n",
    parent_cohort % poly_cohort_size,
    outer
  );
#endif

  // Unshuffle within polyonential cohort
  unshuf = acy_rev_cohort_shuffle(outer, lower_cohort_size, seed);

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::unshuf-in-super::%lu\n",
    unshuf
  );
#endif

  // Escape from polyonential cohort
  id child = acy_multipoly_cohort_outer(
    parent_cohort / poly_cohort_size,
    unshuf,
    poly_cohort_shape,
    lower_cohort_size,
    poly_cohort_layers,
    seed
  );

#ifdef DEBUG_SELECT
  fprintf(
    stderr,
    "select_poly_nth_child::super/result::%lu/%lu\n",
    parent_cohort/poly_cohort_size,
    child
  );
#endif

  /*
  // Adjust index to be >= parent:
  id adjusted = child + parent - acy_select_poly_earliest_possible_child(
    parent,
    avg_arity,
    max_arity,
    poly_cohort_size,
    poly_cohort_layers
  );

  if (adjusted < child) { // overflow
    return NONE;
  }
  */
  // TODO: DEBUG
  id adjusted = child;

  return adjusted;
}
