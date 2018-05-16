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
void acy_select_parent_and_index(
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
id acy_select_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  id seed
);

// Given a parent, returns the number of children that that parent will have
// using acy_select_nth_child. Returns 0 for out-of-bounds values.
id acy_count_select_children(
  id parent,
  id avg_arity,
  id max_arity,
  id seed
);

// For exponential cohort selection (see below) returns the earliest possible
// child of the given parent.
id acy_select_exp_earliest_possible_child(
  id parent,
  id avg_arity,
  id max_arity,
  id exp_cohort_size,
  id exp_cohort_layers
);

// Same as acy_select_exp_earliest_possible_child, but from the child's
// perspective.
id acy_select_exp_child_cohort_start(
  id child,
  id avg_arity,
  id max_arity,
  id exp_cohort_size,
  id exp_cohort_layers
);

// Works like acy_select_parent_and_index, but uses an exponential cohort for
// child selection. The exp_cohort_size parameter controls how large this
// exponential cohort is, in terms of multiples of max_arity. Returns values
// via the r_parent and r_index parameters.
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
);

// Works like acy_select_nth_child, but uses an exponential cohort for
// children. Note that avg_arity is only roughly respected.
id acy_select_exp_nth_child(
  id parent,
  id nth,
  id avg_arity,
  id max_arity,
  double exp_cohort_shape,
  id exp_cohort_size,
  id exp_cohort_layers,
  id seed
);

// For polynomial cohort selection (see below) returns the earliest possible
// child of the given parent.
id acy_select_poly_earliest_possible_child(
  id parent,
  id parent_cohort_size,
  id child_cohort_size,
  id poly_cohort_base,
  id poly_cohort_shape,
  id seed
);

// Same as acy_select_poly_earliest_possible_child, but from the child's
// perspective.
id acy_select_poly_child_cohort_start(
  id child,
  id poly_cohort_base,
  id poly_cohort_shape,
  id seed
);

// Works like acy_select_parent_and_index, but uses a polynomial cohort for
// child selection. The poly_cohort_size parameter controls how large this
// polynomial cohort is, in terms of multiples of max_arity. Returns values
// via the r_parent and r_index parameters.
void acy_select_poly_parent_and_index(
  id child,
  id parent_cohort_size,
  id child_cohort_size,
  id poly_cohort_base,
  id poly_cohort_shape,
  id seed,
  id *r_parent,
  id *r_index
);

// Works like acy_select_nth_child, but uses a polynomial cohort for
// children. Note that avg_arity is only roughly respected.
id acy_select_poly_nth_child(
  id parent,
  id nth,
  id parent_cohort_size,
  id child_cohort_size,
  id poly_cohort_base,
  id poly_cohort_shape,
  id seed
);

// Works like acy_select_parent_and_index, but uses the given tables for child
// selection. The multiplier parameter controls how large the cohort is, along
// with the table given (it is multiplied by child_cohort_size as well).
// Returns values via the r_parent and r_index parameters.
void acy_select_table_parent_and_index(
  id child,
  id parent_cohort_size,
  id child_cohort_size,
  id const * const children_sumtable,
  id children_sumtable_size,
  id table_extra_multiplier,
  id seed,
  id *r_parent,
  id *r_index
);

// Works like acy_select_nth_child, but uses a table-based cohort for
// children. The actual table multiplier used is the child_cohort_size times
// the extra_multiplier.
// Note: if you want parent and child cohort starts to be aligned, the parent
// and child cohort sizes *must* be the same.
id acy_select_table_nth_child(
  id parent,
  id nth,
  id parent_cohort_size,
  id child_cohort_size,
  id const * const children_sumtable,
  id children_sumtable_size,
  id table_extra_multiplier,
  id seed
);

// TODO: Are these correct or even useful?

// For table-based cohort selection (see above) returns the earliest possible
// child of the given parent.
id acy_select_table_earliest_possible_child(
  id parent,
  id parent_cohort_size,
  id child_cohort_size,
  id const * const children_sumtable,
  id children_sumtable_size,
  id children_multiplier,
  id seed
);

// Same as acy_select_table_earliest_possible_child, but from the child's
// perspective.
id acy_select_table_child_cohort_start(
  id child,
  // parent_cohort_size isn't needed here
  id child_cohort_size,
  id const * const children_sumtable,
  id children_sumtable_size,
  id children_multiplier,
  id seed
);

#endif // INCLUDE_SELECT_H
