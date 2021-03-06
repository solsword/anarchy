/**
 * @file: family.c
 *
 * @description: Family relationships definitions.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <stdlib.h> // for malloc
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
  id min_childbearing_age;
  id max_childbearing_age;
  id mother_cohort_size;
  id max_children_per_mother;
  // mother_cohort_size : max_children_per_mother will be the parent : child
  // generation size ratio. Values for this ratio other than 1 will require
  // matching nonlinear birthday assignment schemes (TODO: make those a thing).

  // tabulated cohort parameters:
  id const *birth_age_dist_sumtable;
  id birth_age_dist_sumtable_size;

  // partner parameters:
  id max_partners_per_mother;
    // max_partners_per_mother *must* be smaller than max_children_per_mother
  id likely_partner_age_gap;
  id unlikely_partner_age_gap;
  id min_partner_age;
  id max_partner_age;
  id likely_partner_likelihood; // denominator: n-1/n are likely, 1/n unlikely
  id unlikely_partner_likelihood; // as above for unlikely/full selection
  id multiple_partners_percent;
};
typedef struct acy_family_info_s acy_family_info;

/*************
 * Constants *
 *************/
 
/*
 * The age-of-parent distribution table used to generate the tables below:
 *
 *  id disttable[] = {
 *     1,  1,  2,  3,  9, // at ages 15--19
 *    17, 23, 25, 27, 29, // at ages 20--25
 *    31, 34, 35, 36, 37, // at ages 26--30
 *    38, 39, 40, 39, 37, // at ages 31--35
 *    35, 32, 30, 27, 24, // at ages 36--40
 *    20, 16, 11, 10,  9, // at ages 41--45
 *     7,  4,  2,  1,  1, // at ages 46--50
 *     1,  1,  1,  1,  1, // at ages 51--55
 *  };
 */


id const DEFAULT_BIRTH_AGE_SUMTABLE[41] = { // off-by-one intentional
    0,   1,   2,   3,   4, // at ages 15--19
    5,   6,   7,   9,  13, // at ages 20--24
   20,  29,  39,  50,  66, // at ages 25--29
   86, 110, 137, 167, 199, // at ages 30--34
  234, 271, 310, 350, 389, // at ages 35--39
  427, 464, 500, 535, 569, // at ages 40--44
  600, 629, 656, 681, 704, // at ages 45--49
  721, 730, 733, 735, 736, // at ages 50--54
  737 // overall sum
};

acy_family_info const DEFAULT_FAMILY_INFO = {
  .seed = 9728182391,

  .birth_rate_per_day = 9984, // modern is 350,000+; this is divisible by 32
  .min_childbearing_age = 15 * ONE_EARTH_YEAR, // TODO: Adjust?
  .max_childbearing_age = 55 * ONE_EARTH_YEAR,
  .mother_cohort_size = 32,
  .max_children_per_mother = 32,

  .birth_age_dist_sumtable = DEFAULT_BIRTH_AGE_SUMTABLE,
  .birth_age_dist_sumtable_size = 40, // off-by-one intentional

  .max_partners_per_mother = 16,
  .likely_partner_age_gap = 3 * ONE_EARTH_YEAR,
  .unlikely_partner_age_gap = 7 * ONE_EARTH_YEAR,
  .min_partner_age = 15 * ONE_EARTH_YEAR, // TODO: Swap back
  //.min_partner_age = 20 * ONE_EARTH_YEAR,
  .max_partner_age = 65 * ONE_EARTH_YEAR,
  .likely_partner_likelihood = 6, // 1/6 are unlikely or full
  .unlikely_partner_likelihood = 4, // 1/4 of that 1/6 unlikely are full
  .multiple_partners_percent = 21 // wild guess based on cursory research
};

enum acy_cohort_case_e {
  ACY_COHORT_CASE_LIKELY = 0,
  ACY_COHORT_CASE_UNLIKELY = 1,
  ACY_COHORT_CASE_FULL = 2,
  ACY_COHORT_CASE_SHIFTED = 3,
  ACY_COHORT_CASE_MAX = 4
};
typedef enum acy_cohort_case_e acy_cohort_case;

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
  dst->min_childbearing_age = src->min_childbearing_age;
  dst->max_childbearing_age = src->max_childbearing_age;
  dst->mother_cohort_size = src->mother_cohort_size;
  dst->max_children_per_mother = src->max_children_per_mother;

  dst->birth_age_dist_sumtable_size = src->birth_age_dist_sumtable_size;
  dst->birth_age_dist_sumtable = src->birth_age_dist_sumtable;

  dst->max_partners_per_mother = src->max_partners_per_mother;
  dst->likely_partner_age_gap = src->likely_partner_age_gap;
  dst->unlikely_partner_age_gap = src->unlikely_partner_age_gap;
  dst->min_partner_age = src->min_partner_age;
  dst->max_partner_age = src->max_partner_age;
  dst->likely_partner_likelihood = src->likely_partner_likelihood;
  dst->unlikely_partner_likelihood = src->unlikely_partner_likelihood;
  dst->multiple_partners_percent = src->multiple_partners_percent;
}

void acy_set_info_seed(acy_family_info *info, id seed) {
  info->seed = seed;
}

id acy_get_info_seed(acy_family_info *info) {
  return info->seed;
}

id acy_birthdate(id person, acy_family_info const * const info) {
  return acy_mixed_cohort(person, info->birth_rate_per_day, info->seed + 17);
}

id acy_first_born_on(id day, acy_family_info const * const info) {
  return acy_mixed_cohort_outer(
    day,
    0,
    info->birth_rate_per_day,
    info->seed + 17
  );
}

id acy_get_child_id_adjust(acy_family_info const * const info) {
  return info->birth_rate_per_day * info->min_childbearing_age;
}

id acy_mother(id person, acy_family_info const * const info) {
  id parent, index;
  acy_mother_and_index(person, info, &parent, &index);
  return parent;
}

static inline id acy_family_birth_age_table_multiplier(
  acy_family_info const * const info
) {
  id table_total = acy_table_total(
    info->birth_age_dist_sumtable_size,
    info->birth_age_dist_sumtable
  );
  return (
    (
      info->birth_rate_per_day
    / info->max_children_per_mother
    * ONE_EARTH_YEAR
    ) / table_total
  ) * table_total; // round to nearest multiple of table_total
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

  id multiplier = acy_family_birth_age_table_multiplier(info);

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
  acy_select_table_parent_and_index(
    adjusted,
    info->mother_cohort_size,
    info->max_children_per_mother,
    info->birth_age_dist_sumtable,
    info->birth_age_dist_sumtable_size,
    multiplier,
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
    // Our final index is our index as a 'child' of our mother's duo plus the
    // number of direct children our actual mother has:
    index += acy_count_select_table_children(
      *r_mother,
      info->mother_cohort_size,
      info->max_children_per_mother,
      info->birth_age_dist_sumtable,
      info->birth_age_dist_sumtable_size,
      multiplier,
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
  id multiplier = acy_family_birth_age_table_multiplier(info);
  id first_count = acy_count_select_table_children(
    person,
    info->mother_cohort_size,
    info->max_children_per_mother,
    info->birth_age_dist_sumtable,
    info->birth_age_dist_sumtable_size,
    multiplier,
    info->seed
  );
#ifdef DEBUG_FAMILY
  fprintf(stderr, "acy_direct_child::first_count::%lu\n", first_count);
#endif
  id child;
  if (nth < first_count) {
    child = acy_select_table_nth_child(
      person,
      nth,
      info->mother_cohort_size,
      info->max_children_per_mother,
      info->birth_age_dist_sumtable,
      info->birth_age_dist_sumtable_size,
      multiplier,
      info->seed
    );
#ifdef DEBUG_FAMILY
    fprintf(stderr, "acy_direct_child::child(first)::%lu\n", child);
#endif
  } else { // get children that would think our duo is their parent:
    child = acy_select_table_nth_child(
      acy_child_bearers_duo(person),
      nth - first_count,
      info->mother_cohort_size,
      info->max_children_per_mother,
      info->birth_age_dist_sumtable,
      info->birth_age_dist_sumtable_size,
      multiplier,
      info->seed
    );
#ifdef DEBUG_FAMILY
    fprintf(stderr, "acy_direct_child::child(second)::%lu\n", child);
#endif
  }
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
  id multiplier = acy_family_birth_age_table_multiplier(info);
  return acy_count_select_table_children(
    person,
    info->mother_cohort_size,
    info->max_children_per_mother,
    info->birth_age_dist_sumtable,
    info->birth_age_dist_sumtable_size,
    multiplier,
    info->seed
  ) + acy_count_select_table_children(
    acy_child_bearers_duo(person),
    info->mother_cohort_size,
    info->max_children_per_mother,
    info->birth_age_dist_sumtable,
    info->birth_age_dist_sumtable_size,
    multiplier,
    info->seed
  );
}

// Helpers for partner cohort sizing:
static inline id acy_family_partner_likely_cohort_size(
  acy_family_info const * const info
) {
#ifdef DEBUG_FAMILY
  fprintf(
    stderr,
    "\nacy_family_partner_likely_cohort_size::mx/mn/brpd/result::"
      "%lu/%lu/%lu/%lu\n\n",
    info->max_partner_age,
    info->min_partner_age,
    info->birth_rate_per_day,
    (
      (info->max_partner_age - info->min_partner_age)
    * info->birth_rate_per_day
    / (2 * 2)
    // account for selection in separate-space and mixed cohort size doubling.
    )
  );
#endif
  return (
    info->likely_partner_age_gap
  * info->birth_rate_per_day
  / (2 * 2)
  // account for selection in separate-space and mixed cohort size doubling.
  );
}

static inline id acy_family_partner_unlikely_cohort_size(
  acy_family_info const * const info
) {
  return (
    info->unlikely_partner_age_gap
  * info->birth_rate_per_day
  / (2 * 2)
  // account for selection in separate-space and mixed cohort size doubling.
  );
}

static inline id acy_family_partner_full_cohort_size(
  acy_family_info const * const info
) {
  return (
    (info->max_partner_age - info->min_partner_age)
  * info->birth_rate_per_day
  / (2 * 2)
  // account for selection in separate-space and mixed cohort size doubling.
  );
}

// Helper for getting cohort case parameters
static inline void acy_get_cohort_case_parameters(
  acy_cohort_case cohort_case,
  acy_family_info const * const info,
  id *r_cohort_size,
  id *r_cohort_adjust,
  id *r_cohort_fraction
) {
  id likely_cohort_size = acy_family_partner_likely_cohort_size(info);
  id unlikely_cohort_size = acy_family_partner_unlikely_cohort_size(info);
  id full_cohort_size = acy_family_partner_full_cohort_size(info);
  switch (cohort_case) {
    default:
    case ACY_COHORT_CASE_MAX:
#ifdef DEBUG_FAMILY
      fprintf(
        stderr,
        "Error: Cohort case parameters requested for invalid case %u.\n",
        cohort_case
      );
#endif
      *r_cohort_size = 0;
      *r_cohort_adjust = 0;
      *r_cohort_fraction = 0;
      break;
    case ACY_COHORT_CASE_LIKELY:
      *r_cohort_size = likely_cohort_size;
      *r_cohort_adjust = 0;
      *r_cohort_fraction = *r_cohort_size / info->likely_partner_likelihood;
      break;
    case ACY_COHORT_CASE_UNLIKELY:
      *r_cohort_size = unlikely_cohort_size;
      *r_cohort_adjust = 0;
      *r_cohort_fraction = *r_cohort_size / info->unlikely_partner_likelihood;
      break;
    case ACY_COHORT_CASE_FULL:
      *r_cohort_size = full_cohort_size;
      *r_cohort_adjust = 0;
      *r_cohort_fraction = *r_cohort_size;
      break;
    case ACY_COHORT_CASE_SHIFTED:
      *r_cohort_size = likely_cohort_size;
      *r_cohort_adjust = 1;
      *r_cohort_fraction = 0; // no exclusion for backup offset cohort
      break;
  }
}

static inline id acy_num_potential_partners(acy_family_info const * const info){
  // there are four possible cohort cases
  return 4 * info->max_partners_per_mother;
}

void acy_nth_potential_partner_and_index(
  id person,
  id nth,
  acy_family_info const * const info,
  id *r_partner,
  id *r_nth
) {
  if (acy_is_child_bearer(person)) {
    // child-bearers only have real partners
    *r_partner = NONE;
    *r_nth = 0;
    return;
  }
  acy_cohort_case cohort_case = nth / info->max_partners_per_mother;
  id which_partner = nth % info->max_partners_per_mother;

  // Get case parameters:
  id cohort_size, cohort_adjust, cohort_fraction;
  acy_get_cohort_case_parameters(
    cohort_case,
    info,
    &cohort_size,
    &cohort_adjust,
    &cohort_fraction
  );

  // Find match:
  id cohort, inner;
  acy_mixed_cohort_and_inner(
    acy_separated(person),
    cohort_size,
    info->seed + (83923 * which_partner),
    &cohort,
    &inner
  );
  if (inner < cohort_fraction) {
    // a fractionated partner can't be chosen
    *r_partner = NONE;
    *r_nth = 0;
    return;
  }
  cohort += cohort_adjust; // adjust cohort to correct age gap
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
  id unsep = acy_sep_child_bearer(candidate);
  id num_actual = acy_num_partners(unsep, info);
  if (which_partner >= num_actual) {
    // that other person doesn't have enough partners to include us
    *r_partner = NONE;
    *r_nth = 0;
    return;
  }
  // which nth partner match did this parent start on:
  id start = (unsep + info->seed) % num_actual;
  // subtract without overflow:
  id adj_which = (which_partner + num_actual - start) % num_actual;

  // Set output variables:
  *r_partner = unsep;
  *r_nth = adj_which;
}

id acy_num_partners(id person, acy_family_info const * const info) {
  id partner_count = 0;
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
    id candidate = NONE;
    id partner_index = 0;
    id num_potential = acy_num_potential_partners(info);
    for (id nth = 0; nth < num_potential; ++nth) {
      acy_nth_potential_partner_and_index(
        person,
        nth,
        info,
        &candidate,
        &partner_index
      );
      if (candidate == NONE) {
        continue;
      }
      id check = acy_nth_partner(candidate, partner_index, info);
      if (check == person) {
        partner_count += 1;
      }
    }
    return partner_count;
  }
}

id acy_nth_partner(id person, id nth, acy_family_info const * const info) {
  if (acy_is_child_bearer(person)) {
    id child_count = acy_num_direct_children(person, info);
    if (nth >= child_count) {
      return NONE;
    }
    id num_partners = acy_num_partners(person, info);
    // Note: child ages are unrelated to child ordering, so scrambling partners
    // here won't hurt partner continuity. Actually improving partner
    // continuity would be a good thing of course :) Also note: adding the
    // person and seed ensures that non-child-bearers can have multiple single
    // children with different mothers.
    id which_partner = (nth + person + info->seed) % num_partners;
    id candidate;
    id cohort, inner;
    for (
      acy_cohort_case cohort_case = ACY_COHORT_CASE_LIKELY;
      cohort_case < ACY_COHORT_CASE_MAX;
      ++cohort_case
    ) {
      // Get case parameters:
      id cohort_size, cohort_adjust, cohort_fraction;
      acy_get_cohort_case_parameters(
        cohort_case,
        info,
        &cohort_size,
        &cohort_adjust,
        &cohort_fraction
      );

      // find match:
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
      if (shuf < cohort_fraction) {
        continue; // look in another cohort case
      }

      // which nth partner match did this parent start on:
      id start = (person + info->seed) % child_count;

      // consider partner/parent age gap for each child with this partner:
      id sep_match = acy_mixed_cohort_outer(
        cohort,
        shuf,
        cohort_size,
        info->seed + (83923 * which_partner)
      );
      candidate = acy_sep_non_child_bearer(sep_match);
      for (
        id child_index = start;
        child_index < child_count;
        child_index += num_partners
      ) {
        id child = acy_direct_child(person, child_index, info);
        if (
          acy_birthdate(candidate, info) - acy_birthdate(child, info)
        < info->min_partner_age
        ) {
#ifdef DEBUG_FAMILY
          if (cohort_case == ACY_COHORT_CASE_SHIFTED) {
            fprintf(
              stderr,
              "Error; even shifting wasn't enough to ensure partner age!\n"
              "  Child-bearer: %lu  Shifted partner: %lu  Child: %lu\n"
              "  Birthdates:   %lu                   %lu         %lu\n",
              person, candidate, child,
              acy_birthdate(person, info),
              acy_birthdate(candidate, info),
              acy_birthdate(child, info)
            );
          }
#endif
          continue; // partner is too young; look in another cohort case
        }
      }
    }
    return candidate;
  } else {
    id candidate = NONE;
    id partner_index = 0;
    id num_potential = acy_num_potential_partners(info);
    for (id any = 0; any < num_potential; ++any) {
      acy_nth_potential_partner_and_index(
        person,
        any,
        info,
        &candidate,
        &partner_index
      );
      if (candidate == NONE) {
        continue;
      }
      id check = acy_nth_partner(candidate, partner_index, info);
      if (check == person) {
        nth -= 1;
        if (nth == 0) {
          return candidate;
        }
      }
    }
    // nth is too large or the for loop above failed somehow
    return NONE;
  }
}

id acy_child(id person, id nth, acy_family_info const * const info) {
  if (acy_is_child_bearer(person)) {
    return acy_direct_child(person, nth, info);
  } else {
    id candidate = NONE;
    id partner_index = 0;
    id num_potential = acy_num_potential_partners(info);
    for (id any = 0; any < num_potential; ++any) {
      acy_nth_potential_partner_and_index(
        person,
        any,
        info,
        &candidate,
        &partner_index
      );
      if (candidate == NONE) {
        continue;
      }
      id check = acy_nth_partner(candidate, partner_index, info);
      if (check == person) {
        id num_partners = acy_num_partners(candidate, info);
        id child_count = acy_num_direct_children(candidate, info);
        // figure out how many children we had with this partner:
        id children_with_this_partner = child_count / num_partners;
        // don't forget about leftovers in the mod math:
        id leftovers = child_count - num_partners * children_with_this_partner;
        if (partner_index < leftovers) {
          children_with_this_partner += 1;
        }
        if (nth < children_with_this_partner) { // we've found the right match:
          return acy_direct_child(
            candidate,
            num_partners * nth + partner_index,
            info
          );
        }
        // otherwise there weren't enough children with that partner:
        nth -= children_with_this_partner;
      }
    }
  }
  // Didn't find anyone who both selected us as their Nth partner and had at
  // least N partners total (takes O(max_partners_per_mother) iterations).
  return NONE;
}

id acy_num_children(id person, acy_family_info const * const info) {
  if (acy_is_child_bearer(person)) {
    return acy_num_direct_children(person, info);
  } else {
    id total_children = 0;
    id candidate = NONE;
    id partner_index = 0;
    id num_potential = acy_num_potential_partners(info);
    for (id any = 0; any < num_potential; ++any) {
      acy_nth_potential_partner_and_index(
        person,
        any,
        info,
        &candidate,
        &partner_index
      );
      if (candidate == NONE) {
        continue;
      }
      id check = acy_nth_partner(candidate, partner_index, info);
      if (check == person) {
        id child_count = acy_num_direct_children(candidate, info);
        id num_partners = acy_num_partners(candidate, info);
        // which nth partner match did this parent start on:
        id start = (candidate + info->seed) % num_partners;
        // subtract without overflow:
        id adj_which = (partner_index + num_partners - start) % num_partners;
        // figure out how many children we had with this partner:
        id children_with_this_partner = child_count / num_partners;
        // don't forget about leftovers in the mod math:
        id leftovers = child_count - num_partners * children_with_this_partner;
        if (adj_which < leftovers) {
          children_with_this_partner += 1;
        }
        total_children += children_with_this_partner;
      }
    }
    return total_children;
  }
}
