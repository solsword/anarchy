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

#include "core/unit.h" // for "id" and unit operations

/*************
 * Functions *
 *************/

// Identifies a parent, as well as which child of that parent the child is.
// Returns these values via the r_parent and r_index parameters.
void myc_select_parent_and_index(
  id child,
  id avg_arity,
  id max_arity,
  id seed,
  id *r_parent,
  id *r_index
);

// Given a parent and an index, returns the id of that child. If the index is
// out-of-bounds, it returns NONE. Note that the given max arity establishes a
// cohort of children to be divided between a number of parents such that on
// average each parent with have avg_arity children, but integer divisions mean
// that avg_arity is only roughly respected.
id myc_select_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  id seed
);

#endif // INCLUDE_SELECT_H
