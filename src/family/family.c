/**
 * @file: family.c
 *
 * @description: Family relationships definitions.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <malloc.h>

#include "family.h"

/*************************
 * Structure Definitions *
 *************************/

struct myc_family_info_s {
  id birth_rate_per_day;
  id minimum_age_at_first_child;
  id childbearing_days;
  id average_children_per_mother; // so that population is stable TODO: gender!?
  id max_children_per_mother;
  float child_age_distribution_shape;
  id seed;
};
typedef struct myc_family_info_s myc_family_info;

/*************
 * Constants *
 *************/

myc_family_info const DEFAULT_FAMILY_INFO = {
  .birth_rate_per_day = 10000,
  .minimum_age_at_first_child = 15 * ONE_EARTH_YEAR,
  .childbearing_days = 20 * ONE_EARTH_YEAR,
  .average_children_per_mother = 1,
  .max_children_per_mother = 32,
  .child_age_distribution_shape = 0.01,
  .seed = 9728182391
};

/*************
 * Functions *
 *************/

myc_family_info *myc_create_family_info() {
  return (myc_family_info*) malloc(sizeof(myc_family_info));
}

// Destroys a heap-allocated family info object.
void myc_destroy_family_info(myc_family_info *info) {
  free(info);
}

void myc_copy_family_info(
  myc_family_info const * const src,
  myc_family_info *dst
) {
  dst->birth_rate_per_day = src->birth_rate_per_day;
  dst->minimum_age_at_first_child = src->minimum_age_at_first_child;
  dst->childbearing_days = src->childbearing_days;
  dst->average_children_per_mother = src->average_children_per_mother;
  dst->max_children_per_mother = src->max_children_per_mother;
  dst->child_age_distribution_shape = src->child_age_distribution_shape;
  dst->seed = src->seed;
}

void myc_set_info_seed(myc_family_info *info, id seed) {
  info->seed = seed;
}

id myc_birthdate(id person, myc_family_info const * const info) {
  return myc_mixed_cohort(person, info->birth_rate_per_day, 0);
}

id myc_first_born_on(id day, myc_family_info const * const info) {
  return myc_mixed_cohort_outer(day, 0, info->birth_rate_per_day, 0);
}

id myc_mother(id person, myc_family_info const * const info) {
  id parent, index;
  if (person == NONE) {
    return NONE;
  }
  myc_select_exp_parent_and_index(
    person - (info->birth_rate_per_day * info->minimum_age_at_first_child),
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    info->birth_rate_per_day * info->childbearing_days,
    info->seed,
    &parent,
    &index
  );
  return parent;
}

void myc_mother_and_index(
  id person,
  myc_family_info const * const info,
  id *r_mother,
  id *r_index
) {
  if (person == NONE) {
    *r_mother = NONE;
    *r_index = 0;
    return;
  }
  myc_select_exp_parent_and_index(
    person - (info->birth_rate_per_day * info->minimum_age_at_first_child),
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    info->birth_rate_per_day * info->childbearing_days,
    info->seed,
    r_mother,
    r_index
  );
}

id myc_child(id person, id nth, myc_family_info const * const info) {
  id child = myc_select_exp_nth_child(
    person,
    nth,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    info->birth_rate_per_day * info->childbearing_days,
    info->seed
  );
  if (child == NONE) { return NONE; }
  return child + (info->birth_rate_per_day * info->minimum_age_at_first_child);
}
