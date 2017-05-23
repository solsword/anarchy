/**
 * @file: family.c
 *
 * @description: Family relationships definitions.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <malloc.h>
#ifdef DEBUG_FAMILY
  #include <stdio.h>
#endif

#include "family.h"

/*************************
 * Structure Definitions *
 *************************/

struct acy_family_info_s {
  // the seed:
  id seed;

  // mother/child parameters:
  id birth_rate_per_day;
  id min_age_at_first_child;
  id childbearing_days;
  id average_children_per_mother; // so that population is stable TODO: gender!?
  // TODO: Deal with the doubling of this value!
  id max_children_per_mother;
  double child_age_distribution_shape; // TODO: Still use this?

  // partner parameters:
  id min_partner_age;
  id max_partner_age;
  id multiple_partners_percent;
};
typedef struct acy_family_info_s acy_family_info;

/*************
 * Constants *
 *************/

acy_family_info const DEFAULT_FAMILY_INFO = {
  .seed = 9728182391,

  .birth_rate_per_day = 10000,
  .min_age_at_first_child = 15 * ONE_EARTH_YEAR,
  .childbearing_days = 25 * ONE_EARTH_YEAR,
  .average_children_per_mother = 1,
  .max_children_per_mother = 32,
  .child_age_distribution_shape = 10.0,

  .min_partner_age = 15 * ONE_EARTH_YEAR,
  .max_partner_age = 40 * ONE_EARTH_YEAR, // low for motherhood TODO: fix this?
  .multiple_partners_percent = 21 // wild guess based on cursory research
};

/*************
 * Functions *
 *************/

acy_family_info *acy_create_family_info() {
  return (acy_family_info*) malloc(sizeof(acy_family_info));
}

// Destroys a heap-allocated family info object.
void acy_destroy_family_info(acy_family_info *info) {
  free(info);
}

void acy_copy_family_info(
  acy_family_info const * const src,
  acy_family_info *dst
) {
  dst->seed = src->seed;
  dst->birth_rate_per_day = src->birth_rate_per_day;
  dst->min_age_at_first_child = src->min_age_at_first_child;
  dst->childbearing_days = src->childbearing_days;
  dst->average_children_per_mother = src->average_children_per_mother;
  dst->max_children_per_mother = src->max_children_per_mother;
  dst->child_age_distribution_shape = src->child_age_distribution_shape;

  dst->min_partner_age = src->min_partner_age;
  dst->max_partner_age = src->max_partner_age;
  dst->multiple_partners_percent = src->multiple_partners_percent;
}

void acy_set_info_seed(acy_family_info *info, id seed) {
  info->seed = seed;
}

id acy_get_info_seed(acy_family_info *info) {
  return info->seed;
}

id acy_birthdate(id person, acy_family_info const * const info) {
  return acy_mixed_cohort(person, info->birth_rate_per_day, 0);
}

id acy_first_born_on(id day, acy_family_info const * const info) {
  return acy_mixed_cohort_outer(day, 0, info->birth_rate_per_day, 0);
}

id acy_get_child_id_adjust(acy_family_info const * const info) {
  return info->birth_rate_per_day * info->min_age_at_first_child;
}

id acy_mother(id person, acy_family_info const * const info) {
  id parent, index;
  acy_mother_and_index(person, info, &parent, &index);
  return parent;
}

static inline id acy_family_children_cohort_size(
  acy_family_info const * const info
) {
  return (
    (info->birth_rate_per_day * info->childbearing_days)
  / (info->max_children_per_mother)
  );
}

void acy_mother_and_index(
  id person,
  acy_family_info const * const info,
  id *r_mother,
  id *r_index
) {
#ifdef DEBUG_FAMILY
  fprintf(stderr, "\nacy_mother_and_index::person::%lu\n", person);
#endif
  if (person == NONE) {
    *r_mother = NONE;
    *r_index = 0;
    return;
  }

  // Correct age gap:
  id adjusted = person - acy_get_child_id_adjust(info);
  /*
   * TODO: Check underflow!
  if (adjusted > person) { // underflow
    *r_mother = NONE;
    *r_index = 0;
    return;
  }
  */
#ifdef DEBUG_FAMILY
  fprintf(stderr, "acy_mother_and_index::adjusted::%lu\n", adjusted);
#endif
  id mother, index;
  acy_select_parent_and_index(
    adjusted,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->seed,
    &mother,
    &index
  );
#ifdef DEBUG_FAMILY
  fprintf(
    stderr,
    "acy_mother_and_index::mother/index::%lu/%lu\n",
    mother,
    index
  );
#endif

  *r_mother = acy_child_bearer(mother);
#ifdef DEBUG_FAMILY
  fprintf(stderr, "acy_mother_and_index::adjusted_mother::%lu\n", *r_mother);
#endif
  if (mother != *r_mother) {
    index += acy_count_select_children(
      *r_mother,
      info->average_children_per_mother,
      info->max_children_per_mother,
      info->seed
    );
#ifdef DEBUG_FAMILY
    fprintf(stderr, "acy_mother_and_index::adjusted_index::%lu\n", index);
#endif
  }
#ifdef DEBUG_FAMILY
    fprintf(stderr, "\n");
#endif
  *r_index = index;

  /*
   * TODO: Exponential selection?
  id cohort_size = acy_family_children_cohort_size(info);
  acy_select_exp_parent_and_index(
    adjusted,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    cohort_size,
    FAMILY_COHORT_LAYERS,
    info->seed,
    r_mother,
    r_index
  );
  */
  /*
   * TODO: Check underflow
  if (*r_mother > person) { // underflow
    *r_mother = NONE;
    *r_index = 0;
    return;
  }
  */
}

id acy_direct_child(id person, id nth, acy_family_info const * const info) {
#ifdef DEBUG_FAMILY
  fprintf(stderr, "\nacy_direct_child::person/nth::%lu/%lu\n", person, nth);
#endif
  if (!acy_is_child_bearer(person)) {
    return NONE;
  }
  id first_count = acy_count_select_children(
    person,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->seed
  );
#ifdef DEBUG_FAMILY
  fprintf(stderr, "acy_direct_child::first_count::%lu\n", first_count);
#endif
  id child;
  if (nth < first_count) {
    child = acy_select_nth_child(
      person,
      nth,
      info->average_children_per_mother,
      info->max_children_per_mother,
      info->seed
    );
#ifdef DEBUG_FAMILY
    fprintf(stderr, "acy_direct_child::child(first)::%lu\n", child);
#endif
  } else { // get children that would think our duo is their parent:
    child = acy_select_nth_child(
      person + 1,
      nth - first_count,
      info->average_children_per_mother,
      info->max_children_per_mother,
      info->seed
    );
#ifdef DEBUG_FAMILY
    fprintf(stderr, "acy_direct_child::child(second)::%lu\n", child);
#endif
  }
  /*
   * TODO: Exponential selection (or just do multi-selection?)
  id cohort_size = acy_family_children_cohort_size(info);
  id child = acy_select_exp_nth_child(
    person,
    nth,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    cohort_size,
    FAMILY_COHORT_LAYERS,
    info->seed
  );
  */
  if (child == NONE) { return NONE; } // mother doesn't have this many children
  // Introduce age gap:
  id adjusted = child + acy_get_child_id_adjust(info);
#ifdef DEBUG_FAMILY
  fprintf(stderr, "acy_direct_child::adjusted::%lu\n\n", adjusted);
#endif
  /*
   * TODO: Check overflow
  if (adjusted < child || child < person) { return NONE; } // overflow
  */
  return adjusted;
}

id acy_num_direct_children(id person, acy_family_info const * const info) {
  if (person == NONE || !acy_is_child_bearer(person)) {
    return 0;
  }
  return acy_count_select_children(
    person,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->seed
  ) + acy_count_select_children(
    person + 1,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->seed
  );
}

// Helper for cohort sizing
static inline id acy_family_partner_cohort_size(
  acy_family_info const * const info
) {
#ifdef DEBUG_FAMILY
  fprintf(
    stderr,
    "\nacy_family_partner_cohort_size::mx/mn/brpd/result::%lu/%lu/%lu/%lu\n\n",
    info->max_partner_age,
    info->min_partner_age,
    info->birth_rate_per_day,
    (
      (info->max_partner_age - info->min_partner_age)
    * info->birth_rate_per_day
    / 2
    )
  );
#endif
  return (
    (info->max_partner_age - info->min_partner_age)
  * info->birth_rate_per_day
  / 2 // account for selecting only non-child-bearing persons
  );
}

id acy_num_partners(id person, acy_family_info const * const info) {
  id cohort_size = acy_family_partner_cohort_size(info);
  if (acy_is_child_bearer(person)) {
    id child_count = acy_num_direct_children(person, info);
    id num_partners = 1;
    id random = acy_prng(person, info->seed + 48935729874918238);
    while (
      random % 100 < info->multiple_partners_percent
   && num_partners < child_count
    ) {
      num_partners += 1;
      random = acy_prng(random, info->seed + 48935729874918238 + num_partners);
    }
    return num_partners;
  } else {
    id partner_count = 0;
    for ( // consider every possible match in order
      id which_partner = 0;
      which_partner < info->max_children_per_mother;
      ++which_partner
    ) {
      id cohort, inner;
      acy_mixed_cohort_and_inner(
        acy_separated(person),
        cohort_size,
        info->seed + (83923 * which_partner),
        &cohort,
        &inner
      );
      id unshuf = acy_rev_cohort_shuffle(
        inner,
        cohort_size,
        info->seed + (28999 * which_partner)
      );
      id candidate = acy_mixed_cohort_outer(
        cohort,
        unshuf,
        cohort_size,
        info->seed + (83923 * which_partner)
      );
      id full_candidate = acy_sep_child_bearer(candidate);
      id num_partners = acy_num_partners(full_candidate, info);
      // count up if we actually have children with this candidate:
      if (which_partner < num_partners) {
        partner_count += 1;
      }
    }
    return partner_count;
  }
}

id acy_nth_partner(id person, id nth, acy_family_info const * const info) {
  id cohort_size = acy_family_partner_cohort_size(info);
#ifdef DEBUG_FAMILY
  fprintf(stderr, "\nacy_nth_partner::FPCS::%lu\n\n", cohort_size);
#endif
  if (acy_is_child_bearer(person)) {
    id child_count = acy_num_direct_children(person, info);
    if (nth > child_count) {
      return NONE;
    }
    id num_partners = acy_num_partners(person, info);
    // Note: child ages are essentially random anyways, so scrambling partners
    // here won't hurt partner continuity. Actually improving partner
    // continuity would be a good thing of course :)
    // Also note: adding the person and seed ensures that non-child-bearers can
    // have multiple single children with different mothers.
    id which_partner = (nth + person + info->seed) % num_partners;
    id cohort, inner;
    acy_mixed_cohort_and_inner(
      acy_separated(person),
      cohort_size,
      info->seed + (1827 * which_partner),
      &cohort,
      &inner
    );
    id shuf = acy_cohort_shuffle(
      inner,
      cohort_size,
      info->seed + (28999 * which_partner)
    );
    id match = acy_mixed_cohort_outer(
      cohort,
      shuf,
      cohort_size,
      info->seed + (83923 * which_partner)
    );
    return acy_sep_non_child_bearer(match);
  } else {
    for ( // consider every possible match in order
      id which_partner = 0;
      which_partner < info->max_children_per_mother;
      ++which_partner
    ) {
      id cohort, inner;
      acy_mixed_cohort_and_inner(
        acy_separated(person),
        cohort_size,
        info->seed + (83923 * which_partner),
        &cohort,
        &inner
      );
      id unshuf = acy_rev_cohort_shuffle(
        inner,
        cohort_size,
        info->seed + (28999 * which_partner)
      );
      id candidate = acy_mixed_cohort_outer(
        cohort,
        unshuf,
        cohort_size,
        info->seed + (83923 * which_partner)
      );
      id full_candidate = acy_sep_child_bearer(candidate);
      id child_count = acy_num_direct_children(full_candidate, info);
      id num_partners = acy_num_partners(full_candidate, info);
      if (which_partner >= num_partners) {
        continue; // no children with this potential partner
      }
      // which nth partner match did this parent start on:
      id start = (full_candidate + info->seed) % num_partners;
      // subtract without overflow:
      id adj_which = (which_partner + num_partners - start) % num_partners;
      // figure out how many children we had with this partner:
      id children_with_this_partner = child_count / num_partners;
      // don't forget about leftovers in the mod math:
      id leftovers = child_count - num_partners * children_with_this_partner;
      if (adj_which < leftovers) {
        children_with_this_partner += 1;
      }
      if (nth < children_with_this_partner) { // we've found the right match:
        return full_candidate;
      }
      // otherwise there weren't enough children with that partner:
      nth -= children_with_this_partner;
    }
    // Didn't find anyone who both selected us as their Nth partner and had at
    // least N partners total (takes O(max_children) iterations).
    return NONE;
  }
}

id acy_child(id person, id nth, acy_family_info const * const info) {
  if (acy_is_child_bearer(person)) {
    return acy_direct_child(person, nth, info);
  } else {
    id cohort_size = acy_family_partner_cohort_size(info);
    for ( // consider every possible match in order
      id which_partner = 0;
      which_partner < info->max_children_per_mother;
      ++which_partner
    ) {
      id cohort, inner;
      acy_mixed_cohort_and_inner(
        acy_separated(person),
        cohort_size,
        info->seed + (83923 * which_partner),
        &cohort,
        &inner
      );
      id unshuf = acy_rev_cohort_shuffle(
        inner,
        cohort_size,
        info->seed + (28999 * which_partner)
      );
      id candidate = acy_mixed_cohort_outer(
        cohort,
        unshuf,
        cohort_size,
        info->seed + (83923 * which_partner)
      );
      id full_candidate = acy_sep_child_bearer(candidate);
      id child_count = acy_num_direct_children(full_candidate, info);
      id num_partners = acy_num_partners(full_candidate, info);
      if (which_partner >= num_partners) {
        continue; // no children with this potential partner
      }
      // which nth partner match did this parent start on:
      id start = (full_candidate + info->seed) % num_partners;
      // subtract without overflow:
      id adj_which = (which_partner + num_partners - start) % num_partners;
      // figure out how many children we had with this partner:
      id children_with_this_partner = child_count / num_partners;
      // don't forget about leftovers in the mod math:
      id leftovers = child_count - num_partners * children_with_this_partner;
      if (adj_which < leftovers) {
        children_with_this_partner += 1;
      }
      if (nth < children_with_this_partner) { // we've found the right match:
        return acy_direct_child(
          full_candidate,
          num_partners * nth + adj_which,
          info
        );
      }
      // otherwise there weren't enough children with that partner:
      nth -= children_with_this_partner;
    }
    // Didn't find anyone who both selected us as their Nth partner and had at
    // least N partners total (takes O(max_children) iterations).
    return NONE;
  }
}

id acy_num_children(id person, acy_family_info const * const info) {
  if (acy_is_child_bearer(person)) {
    return acy_num_direct_children(person, info);
  } else {
    id cohort_size = acy_family_partner_cohort_size(info);
    id total_children = 0;
    for ( // consider every possible match in order
      id which_partner = 0;
      which_partner < info->max_children_per_mother;
      ++which_partner
    ) {
      id cohort, inner;
      acy_mixed_cohort_and_inner(
        acy_separated(person),
        cohort_size,
        info->seed + (83923 * which_partner),
        &cohort,
        &inner
      );
      id unshuf = acy_rev_cohort_shuffle(
        inner,
        cohort_size,
        info->seed + (28999 * which_partner)
      );
      id candidate = acy_mixed_cohort_outer(
        cohort,
        unshuf,
        cohort_size,
        info->seed + (83923 * which_partner)
      );
      id full_candidate = acy_sep_child_bearer(candidate);
      id child_count = acy_num_direct_children(full_candidate, info);
      id num_partners = acy_num_partners(full_candidate, info);
      if (which_partner >= num_partners) {
        continue; // no children with this potential partner
      }
      // which nth partner match did this parent start on:
      id start = (full_candidate + info->seed) % num_partners;
      // subtract without overflow:
      id adj_which = (which_partner + num_partners - start) % num_partners;
      // figure out how many children we had with this partner:
      id children_with_this_partner = child_count / num_partners;
      // don't forget about leftovers in the mod math:
      id leftovers = child_count - num_partners * children_with_this_partner;
      if (adj_which < leftovers) {
        children_with_this_partner += 1;
      }
      total_children += children_with_this_partner;
    }
    return total_children;
  }
}
