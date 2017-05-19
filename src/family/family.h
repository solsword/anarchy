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

// Returns the nth child of the given person. Returns NONE if that person
// doesn't have that many children.
id acy_child(id person, id nth, acy_family_info const * const info);

#endif // INCLUDE_FAMILY_H
