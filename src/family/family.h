/**
 * @file: family.h
 *
 * @description: Family relationships definitions.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#ifndef INCLUDE_FAMILY_H
#define INCLUDE_FAMILY_H

#include "core/unit.h" // for id type
#include "core/cohort.h" // for acy_mixed_cohort
#include "core/select.h" // for acy_select_exp_parent_and_index

/**************************
 * Structure Declarations *
 **************************/

struct acy_family_info_s;
typedef struct acy_family_info_s acy_family_info;

/*************
 * Constants *
 *************/

// Days in a year
#define ONE_EARTH_YEAR 365

// Default family parameters
extern acy_family_info const DEFAULT_FAMILY_INFO;

/********************
 * Inline Functions *
 ********************/

// Functions to select the child-bearing/non-child-bearing half of a duo, using
// either separated or non-separated IDs.
static inline id acy_child_bearer(id non_normalized) {
  return 2 * (non_normalized / 2);
}

static inline id acy_sep_child_bearer(id normalized) {
  return 2 * normalized;
}

static inline id acy_non_child_bearer(id non_normalized) {
  return 2 * (non_normalized / 2) + 1;
}

static inline id acy_sep_non_child_bearer(id normalized) {
  return 2 * normalized + 1;
}

// Takes a full person ID and returns the corresponding separated ID for child
// bearer / non-child-bearer separation.
static inline int acy_separated(id person) {
  return person / 2;
}

// Whether or not someone is a child-bearer.
static inline int acy_is_child_bearer(id non_normalized) {
  return non_normalized % 2 == 0;
}

/*************
 * Functions *
 *************/

// Creates a new acy_family_info object on the heap.
acy_family_info *acy_create_family_info();

// Destroys a heap-allocated family info object.
void acy_destroy_family_info(acy_family_info *info);

// Copies family info from src into dst.
void acy_copy_family_info(
  acy_family_info const * const src,
  acy_family_info *dst
);

// Get/set the seed of a family info object.
void acy_set_info_seed(acy_family_info *info, id seed);
id acy_get_info_seed(acy_family_info *info);

// Returns a person's birth date (in days).
id acy_birthdate(id person, acy_family_info const * const info);

// Returns the first person born on the given day.
id acy_first_born_on(id day, acy_family_info const * const info);

// Returns a number used to adjust child IDs to correct age discrepancies.
id acy_get_child_id_adjust(acy_family_info const * const info);

// Returns the mother of the given person.
id acy_mother(id person, acy_family_info const * const info);

// Returns both a person's mother and their index within that mother's
// children. Uses return parameters r_mother and r_index to do so.
void acy_mother_and_index(
  id person,
  acy_family_info const * const info,
  id *r_mother,
  id *r_index
);

// Returns the nth direct child of the given person. Returns NONE if that
// person doesn't have that many direct children. Note that non-child-bearers
// have no direct children (see acy_child below).
id acy_direct_child(id person, id nth, acy_family_info const * const info);

// Returns the number of direct children that the given person has. Note that
// this function only takes into account children borne by this person, so
// non-child-bearers don't have any direct children (see acy_num_children
// below).
id acy_num_direct_children(id person, acy_family_info const * const info);

// Returns the ID of the nth potential partner of a non-child-bearer (always
// returns NONE for child-bearers, who don't have potential partners, just
// actual partners). Each non-child-bearer has 4 * max_partners_per_mother
// potential partners, many of whom aren't actual partners because they didn't
// select that non-child-bearer as a partner. Via return parameters, this
// returns the partner candidate and which partner of that candidate it would
// be.
void acy_nth_potential_partner_and_index(
  id person,
  id nth,
  acy_family_info const * const info,
  id *r_partner,
  id *r_nth
);

// For child-bearers, decides the number of partners that they have. For
// non-child-bearers, counts partners.
id acy_num_partners(id person, acy_family_info const * const info);

// Returns the partner with which this person created their nth child. The
// number of partners for each child-bearer is the same as the number of direct
// children, but the number of partners for non-child-bearers varies
// independently (although of course, the average must still be the same). Note
// that it is impossible for a child-bearer to bear a child with another
// child-bearer. This is a programming shortcut, but is apparently true enough
// to-date even given modern medical technology.
id acy_nth_partner(id person, id nth, acy_family_info const * const info);

// Returns the nth child of the given person, including children borne by
// partners.
id acy_child(id person, id nth, acy_family_info const * const info);

// Returns the true number of children a person has, including children borne
// by partners.
id acy_num_children(id person, acy_family_info const * const info);

#endif // INCLUDE_FAMILY_H
