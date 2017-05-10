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
#include "core/cohort.h" // for myc_mixed_cohort
#include "core/select.h" // for myc_select_exp_parent_and_index

/**************************
 * Structure Declarations *
 **************************/

struct myc_family_info_s;
typedef struct myc_family_info_s myc_family_info;

/*************
 * Constants *
 *************/

// Days in a year
#define ONE_EARTH_YEAR 365

// Default family parameters
extern myc_family_info const DEFAULT_FAMILY_INFO;

/********************
 * Inline Functions *
 ********************/


/*************
 * Functions *
 *************/

// Creates a new myc_family_info object on the heap.
myc_family_info *myc_create_family_info();

// Destroys a heap-allocated family info object.
void myc_destroy_family_info(myc_family_info *info);

// Copies family info from src into dst.
void myc_copy_family_info(
  myc_family_info const * const src,
  myc_family_info *dst
);

// Sets the seed of a family info object.
void myc_set_info_seed(myc_family_info *info, id seed);

// Returns a person's birth date (in days).
id myc_birthdate(id person, myc_family_info const * const info);

// Returns the first person born on the given day.
id myc_first_born_on(id day, myc_family_info const * const info);

// Returns the mother of the given person.
id myc_mother(id person, myc_family_info const * const info);

// Returns both a person's mother and their index within that mother's
// children. Uses return parameters r_mother and r_index to do so.
void myc_mother_and_index(
  id person,
  myc_family_info const * const info,
  id *r_mother,
  id *r_index
);

// Returns the nth child of the given person. Returns NONE if that person
// doesn't have that many children.
id myc_child(id person, id nth, myc_family_info const * const info);

#endif // INCLUDE_FAMILY_H
