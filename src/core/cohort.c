/**
 * @file: cohort.c
 *
 * @description: Reversible operations on cohorts.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include "cohort.h"

void acy_fill_inv_sumtree(
  id *sumtable,
  id table_size,
  id *inv_sumtree
) {
#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\nfill_inv_sumtree_recursive/START::ts::%lu\n",
    table_size
  );
  fprintf(stderr, " ST: ");
  for (id i = 0; i < table_size + 1; ++i) {
    fprintf(stderr, "%lu, ", sumtable[i]);
  }
  fprintf(stderr, "\n");
#endif
  id sts = acy_inv_sumtree_size(table_size);
  id idx = acy_tree_first(sts);
  id table_which = 1;
  id index_which = 0;
  for (id i = 0; i < sts; ++i) {
    if (idx < table_size - 1) { // a sumtable value entry
      inv_sumtree[idx] = sumtable[table_which];
      table_which += 1;
    } else { // an index entry
      inv_sumtree[idx] = index_which;
      index_which += 1;
    }
    idx = acy_tree_next_index(idx, sts);
  }
}

void acy_create_tables(
  id *disttable,
  id table_size,
  id **r_sumtable,
  id **r_inv_sumtree,
  id *r_inv_sumtree_size
) {
  *r_sumtable = (id*) malloc(sizeof(id)*(table_size+1));
  *r_inv_sumtree_size = acy_inv_sumtree_size(table_size);
  *r_inv_sumtree = (id*) malloc(sizeof(id)*(*r_inv_sumtree_size));

  acy_fill_sumtable(
    disttable,
    table_size,
    *r_sumtable
  );

  acy_fill_inv_sumtree(
    *r_sumtable,
    table_size,
    *r_inv_sumtree
  );
}

void acy_cleanup_tables(
  id *sumtable,
  id *inv_sumtree
) {
  free(sumtable);
  free(inv_sumtree);
}

void acy_tabulated_cohort_and_inner(
  id outer,
  id *sumtable,
  id table_size,
  id *inv_sumtree,
  id inv_sumtree_size,
  id multiplier,
  id seed,
  id *r_cohort,
  id *r_inner
) {
  id cohort_size = acy_tablesum(table_size, sumtable) * multiplier;
  id super_size = cohort_size * table_size;

  id super_cohort;
  id super_inner;
  acy_cohort_and_inner(outer, super_size, &super_cohort, &super_inner);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\ntabulated_cohort::outer/multiplier::%lu/%lu\n",
    outer,
    multiplier
  );
  fprintf(
    stderr,
    "tabulated_cohort::tsize/size/super::%lu/%lu/%lu\n",
    table_size,
    cohort_size,
    super_size
  );
  fprintf(
    stderr,
    "tabulated_cohort::super_cohort/inner::%lu/%lu\n",
    super_cohort,
    super_inner
  );
#endif

  // section information:
  id section = super_inner / cohort_size;
  id in_section = super_inner % cohort_size;

  // shuffle within each section
  id shuf = acy_cohort_shuffle(in_section, cohort_size, seed + section);

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_cohort::section/in_section/shuffled::%lu/%lu/%lu\n",
    section,
    in_section,
    shuf
  );
#endif

#ifdef DEBUG_COHORT
  fprintf(stderr, "tabulated_cohort::inv_sumtree:+\n");
  id lim = 1;
  id j = 0;
  for (id k = 0; k < inv_sumtree_size; ++k) {
    fprintf(stderr, "%lu ", inv_sumtree[k]);
    j += 1;
    if (j >= lim) {
      fprintf(stderr, "\n");
      lim *= 2;
      j = 0;
    }
  }
  if (j != 0) {
    fprintf(stderr, "\n");
  }
#endif

  // Figure out which slice we fall into:
  id slice = acy_inv_tablesum(shuf, inv_sumtree, inv_sumtree_size, multiplier);
  // ...and where we are in that slice:
  id after_slice = acy_tablesum(slice, sumtable) * multiplier;
  id in_slice = shuf - after_slice;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_cohort::slice/after_slice/in_slice::%lu/%lu/%lu\n",
    slice,
    after_slice,
    in_slice
  );
#endif

  // We know that there are table_size cohorts per super_cohort, so before
  // the previous super_cohort, there were table_size * (super_cohort - 1)
  // cohorts. Now if we're slice 0 of section 0, we're actually in the very
  // last segment of the second cohort introduced during the previous
  // super_cohort. Similarly, slice 1 of section 0 is the second-to-last
  // segment of the second cohort introduced in the previous super cohort.
  // Additionally, for each section we move over, we're also shifting a cohort.
  // So section + slice + 1 gets added to find the cohort we're in.
  *r_cohort = table_size * (super_cohort - 1) + section + slice + 1;
  // TODO: Detect underflow here?

  // The sum of all things previous to us within our cohort is just the
  // difference of two table sums: the table sum for the full cohort minus the
  // table sum for our segment, and notice that our slice is the same as our
  // segment. So:
  *r_inner = (
    cohort_size - 1
  - (acy_tablesum(slice, sumtable) * multiplier)
  - in_slice
  );

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_cohort::cohort/inner::%lu/%lu\n",
    *r_cohort,
    *r_inner
  );
#endif
}

id acy_tabulated_cohort_outer(
  id cohort,
  id inner,
  id *sumtable,
  id table_size,
  id *inv_sumtree,
  id inv_sumtree_size,
  id multiplier,
  id seed
) {
  id cohort_size = acy_tablesum(table_size, sumtable) * multiplier;
  id super_size = cohort_size * table_size;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\ntabulated_outer::cohort/inner/multiplier::%lu/%lu/%lu\n",
    cohort,
    inner,
    multiplier
  );
  fprintf(
    stderr,
    "tabulated_outer::tsize/size/super::%lu/%lu/%lu\n",
    table_size,
    cohort_size,
    super_size
  );
#endif

  id inv_inner = cohort_size - 1 - inner;

  id segment = acy_inv_tablesum(
    inv_inner,
    inv_sumtree,
    inv_sumtree_size,
    multiplier
  );
  id after = acy_tablesum(segment, sumtable) * multiplier;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer::segment/after::%lu/%lu\n",
    segment,
    after
  );
#endif

  id super_cohort = cohort / table_size;
  // TODO: Double-check this math (-1 in the second half?)!
  id section = (cohort % table_size) + (table_size - segment) - 1;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer::super_cohort/full_section::%lu/%lu\n",
    super_cohort,
    section
  );
#endif

  id in_segment = inv_inner - after;

  if (section >= table_size) {
    super_cohort += 1;
    section -= table_size;
  }

  id shuf = after + in_segment;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer::new_super/section/in_segment/shuffled::%lu/%lu/%lu/%lu\n",
    super_cohort,
    section,
    in_segment,
    shuf
  );
#endif

  id in_section = acy_rev_cohort_shuffle(shuf, cohort_size, seed + section);

  id result = acy_cohort_outer(
    super_cohort,
    section * cohort_size + in_section,
    super_size
  );

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer::inner/result::%lu/%lu\n\n",
    section * cohort_size + in_section,
    result
  );
#endif

  return result;
}

id acy_tabulated_outer_min(
  id cohort,
  id *sumtable,
  id sumtable_size,
  id *inv_sumtree,
  id inv_sumtree_size,
  id multiplier
) {
  id cohort_size = acy_tablesum(sumtable_size - 1, sumtable) * multiplier;
  id super_size = cohort_size * sumtable_size;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "\ntabulated_outer_min::cohort/multiplier::%lu/%lu\n",
    cohort,
    multiplier
  );
  fprintf(
    stderr,
    "tabulated_outer_min::tsize/size/super::%lu/%lu/%lu\n",
    sumtable_size,
    cohort_size,
    super_size
  );
#endif

  id inv_inner = cohort_size - 1;

  id segment = acy_inv_tablesum(
    inv_inner,
    inv_sumtree,
    inv_sumtree_size,
    multiplier
  );

#ifdef DEBUG_COHORT
  fprintf(stderr, "tabulated_outer_min::segment::%lu\n", segment);
#endif

  id super_cohort = cohort / sumtable_size;
  // TODO: Re-copy this math from above if that math changes.
  id section = (cohort % sumtable_size) + (sumtable_size - segment) - 1;

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer_min::super_cohort/full_section::%lu/%lu\n",
    super_cohort,
    section
  );
#endif

  if (section >= sumtable_size) {
    super_cohort += 1;
    section -= sumtable_size;
  }

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer_min::new_super/section::%lu/%lu\n",
    super_cohort,
    section
  );
#endif

  // minimum possible
  id in_section = 0;

  id result = acy_cohort_outer(
    super_cohort,
    section * cohort_size + in_section,
    super_size
  );

#ifdef DEBUG_COHORT
  fprintf(
    stderr,
    "tabulated_outer_min::inner/result::%lu/%lu\n\n",
    section * cohort_size + in_section,
    result
  );
#endif

  return result;
}
