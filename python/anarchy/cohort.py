"""
Reversible & incremental operations on cohorts.

cohort.py

Cohorts are simply sequences of adjacent numbers, taken in chunks of a
fixed size. For example, if the cohort size is 10, then the numbers 0-9
form the first cohort, 10-19 form the second cohort, etc.

These operations do things like shuffle values within cohorts, but are
incremental, in the sense that we can compute the shuffled position of
one cohort member without knowing about all of the rest of them, and
they're reversible, in the sense that we can compute not just where a
value will end up after shuffling, but given a shuffled position, which
value will end up there.
"""

import math

from . import rng


# Basic operations
# ----------------


def cohort(outer, cohort_size):
    """
    Parameters:

    - `outer` (int): An integer to be assigned to a cohort.
    - `cohort_size` (int): The size of each cohort.

    Returns (int): Which cohort the given outer item is assigned to.

    Simply returns the outer index divided by the cohort size, rounded
    down. Note that for extremely large integers, they will be truncated
    to fit within 64 bits.
    """
    return math.floor(outer // cohort_size) & rng.ID_MASK


def cohort_inner(outer, cohort_size):
    """
    Parameters:

    - `outer` (int): An integer to be assigned to a cohort.
    - `cohort_size` (int): The size of each cohort.

    Returns (int): Where within its cohort the given integer ends up.

    Computes within-cohort index for the given outer index and cohorts of
    the given size, which is just the index modulus the cohort size. Like
    `cohort`, truncates to 64 bits.
    """
    return (outer % cohort_size) & rng.ID_MASK


def cohort_and_inner(outer, cohort_size):
    """
    Parameters:

    - `outer` (int): An integer to be assigned to a cohort.
    - `cohort_size` (int): The size of each cohort.

    Returns (length-2 list): A cohort number and within-cohort index.

    Simply returns the results of the `cohort` and `cohort_inner` as a
    pair.
    """
    return [ cohort(outer, cohort_size), cohort_inner(outer, cohort_size) ]


def cohort_outer(cohort, inner, cohort_size):
    """
    Parameters:

    - `cohort` (int): Which cohort the item is in.
    - `inner` (int): The index of the item in that cohort.
    - `cohort_size` (int): The size of each cohort.

    Returns (int): The outer index that would be mapped to this
    within-cohort index.

    This is the Inverse of `cohort_and_inner`: it computes the outer
    index from a cohort number and inner index.
    """
    return (cohort_size * cohort + inner) & rng.ID_MASK


# Shuffling
# ---------

def cohort_interleave(inner, cohort_size):
    """
    Interleaves cohort members by folding the top half into the bottom
    half.
    """
    if inner < (cohort_size + 1) // 2:
        return (inner * 2) & rng.ID_MASK
    else:
        return (((cohort_size - 1 - inner) * 2) + 1) & rng.ID_MASK


def rev_cohort_interleave(inner, cohort_size):
    """
    Inverse of `cohort_interleave`.
    """
    if inner % 2:
        return cohort_size - 1 - inner // 2
    else:
        return inner // 2


def cohort_fold(inner, cohort_size, seed):
    """
    Folds items past an arbitrary split point (in the second half of the
    cohort) into the middle of the cohort. The split will always leave
    an odd number at the end.
    """
    half = cohort_size // 2
    quarter = cohort_size // 4
    split = half
    if quarter > 0:
        split += seed % quarter

    after = cohort_size - split
    split += (after + 1) % 2 # force an odd split point
    after = cohort_size - split

    fold_to = half - after // 2

    if (inner < fold_to): # first region
        return inner & rng.ID_MASK
    elif (inner < split): # second region
        return (inner + after) & rng.ID_MASK # push out past fold region
    else: # fold region
        return (inner - split + fold_to) & rng.ID_MASK


def rev_cohort_fold(inner, cohort_size, seed):
    """
    Inverse of `cohort_fold`.
    """
    half = cohort_size // 2
    quarter = cohort_size // 4
    split = half
    if quarter > 0:
        split += seed % quarter

    after = cohort_size - split
    split += (after + 1) % 2 # force an odd split point
    after = cohort_size - split

    fold_to = half - after // 2

    if (inner < fold_to): # first region
        return inner & rng.ID_MASK
    elif (inner < fold_to + after): # second region
        return (inner - fold_to + split) & rng.ID_MASK
    else:
        return (inner - after) & rng.ID_MASK


def cohort_spin(inner, cohort_size, seed):
    """
    Applies a circular offset
    """
    return ((inner + seed) % cohort_size) & rng.ID_MASK


def rev_cohort_spin(inner, cohort_size, seed):
    """
    Inverse of `cohort_spin`.
    """
    return (
        (inner + (cohort_size - seed % cohort_size))
      % cohort_size
    ) & rng.ID_MASK


def cohort_flop(inner, cohort_size, seed):
    """
    Flops sections (with seeded sizes) with their neighbors.

    Flop is its own inverse.
    """
    limit = cohort_size // 8
    if limit < 4:
        limit += 4

    size = (seed % limit) + 2
    which = inner // size
    local = (inner % size)

    result = 0
    if which % 2:
        result = (which - 1) * size + local
    else:
        result = (which + 1) * size + local

    if (result >= cohort_size): # don't flop out of the cohort
        return inner & rng.ID_MASK
    else:
        return result & rng.ID_MASK


def cohort_mix(inner, cohort_size, seed):
    """
    Applies a spin to both even and odd items with different seeds.
    """
    even = inner - (inner % 2)
    target = 0
    if inner % 2:
        target = cohort_spin(
            even // 2,
            cohort_size // 2,
            seed + 464185
        )
        return (2 * target + 1) & rng.ID_MASK
    else:
        target = cohort_spin(
            even // 2,
            (cohort_size + 1) // 2,
            seed + 1048239
        )
        return (2 * target) & rng.ID_MASK


def rev_cohort_mix(inner, cohort_size, seed):
    """
    Inverse of `cohort_mix`.
    """
    even = inner - (inner % 2)
    target = 0
    if inner % 2:
        target = rev_cohort_spin(
            even // 2,
            cohort_size // 2,
            seed + 464185
        )
        return (2 * target + 1) & rng.ID_MASK
    else:
        target = rev_cohort_spin(
            even // 2,
            (cohort_size + 1) // 2,
            seed + 1048239
        )
        return (2 * target) & rng.ID_MASK


MIN_REGION_SIZE = 2
"""
Minimum size of a region for `cohort_spread`.
"""

MAX_REGION_COUNT = 16
"""
Maximum number of regions for `cohort_spread`.
"""


def cohort_spread(inner, cohort_size, seed):
    """
    Spreads items out between a random number of different regions within
    the cohort.
    """
    min_regions = 2
    if cohort_size < 2 * MIN_REGION_SIZE:
        min_regions = 1

    max_regions = 1 + cohort_size // MIN_REGION_SIZE
    regions = (
        min_regions + (
          (seed % (1 + (max_regions - min_regions)))
        % MAX_REGION_COUNT
        )
    )
    region_size = cohort_size // regions
    leftovers = cohort_size - (regions * region_size)

    region = inner % regions
    index = inner // regions
    if index < region_size: # non-leftovers
        return (region * region_size + index + leftovers) & rng.ID_MASK
    else: # leftovers go at the front:
        return (inner - regions * region_size) & rng.ID_MASK


def rev_cohort_spread(inner, cohort_size, seed):
    """
    Inverse of `cohort_spread`.
    """
    min_regions = 2
    if cohort_size < 2 * MIN_REGION_SIZE:
        min_regions = 1

    max_regions = 1 + cohort_size // MIN_REGION_SIZE
    regions = (
        min_regions + (
          (seed % (1 + (max_regions - min_regions)))
        % MAX_REGION_COUNT
        )
    )

    region_size = cohort_size // regions
    leftovers = cohort_size - (regions * region_size)

    index = (inner - leftovers) // region_size
    region = (inner - leftovers) % region_size

    if inner < leftovers: # leftovers back to the end:
        return (regions * region_size + inner) & rng.ID_MASK
    else:
        return (region * regions + index) & rng.ID_MASK


def cohort_upend(inner, cohort_size, seed):
    """
    Reverses ordering within each of several fragments.

    Upend is its own inverse.
    """
    min_regions = 2
    if cohort_size < 2 * MIN_REGION_SIZE:
        min_regions = 1

    max_regions = 1 + cohort_size // MIN_REGION_SIZE
    regions = (
        min_regions + (
          (seed % (1 + (max_regions - min_regions)))
        % MAX_REGION_COUNT
        )
    )
    region_size = cohort_size // regions

    region = inner // region_size
    index = inner % region_size

    result = (region * region_size) + (region_size - 1 - index)

    if result < cohort_size:
        return result & rng.ID_MASK
    else:
        return inner & rng.ID_MASK


def cohort_shuffle(inner, cohort_size, seed):
    """
    Parameters:

    - `inner` (int): The index of an item in a cohort.
    - `cohort_size` (int): The size of each cohort.
    - `seed` (int): The seed that determines the shuffle order.

    Returns (int): A new within-cohort index for the given item.

    Implements an incremental shuffle, such that if you were to call
    `cohort_shuffle` on each index in a cohort of a given size, each of
    the items would end up in a different new spot within that cohort.
    The shuffle is reversible using the `rev_cohort_shuffle` function.

    It works by composing several of the simpler reversible/indexable
    functions to produce a sufficiently complex operation that humans
    generally won't be able to see the pattern it uses.

    Note that this should *not* be used for cryptography or statistics:
    when the cohort size is even moderately large (say 100), the number
    of possible shuffles (constrained by the seed, which is 64 bits, so
    2^64) will be much smaller than the number of possible orderings of
    the items, so not all possible orderings will be able to be
    produced.

    Both the seed and the cohort size are used to determine the
    particular ordering of items, so the same seed with a different
    cohort size won't necessarily produce a similar ordering to another
    run.
    """
    r = inner
    seed = seed ^ cohort_size
    r = cohort_spread(r, cohort_size, seed + 457) # prime
    r = cohort_mix(r, cohort_size, seed + 2897) # prime
    r = cohort_interleave(r, cohort_size)
    r = cohort_spin(r, cohort_size, seed + 1987) # prime
    r = cohort_upend(r, cohort_size, seed + 47) # prime
    r = cohort_fold(r, cohort_size, seed + 839) # prime
    r = cohort_interleave(r, cohort_size)
    r = cohort_flop(r, cohort_size, seed + 53) # prime
    r = cohort_fold(r, cohort_size, seed + 211) # prime
    r = cohort_mix(r, cohort_size, seed + 733) # prime
    r = cohort_spread(r, cohort_size, seed + 881) # prime
    r = cohort_interleave(r, cohort_size)
    r = cohort_flop(r, cohort_size, seed + 193) # prime
    r = cohort_upend(r, cohort_size, seed + 794641) # prime
    r = cohort_spin(r, cohort_size, seed + 19) # prime
    return r


def rev_cohort_shuffle(inner, cohort_size, seed):
    """
    Parameters:

    - `inner` (int): The index of an item in a cohort.
    - `cohort_size` (int): The size of each cohort.
    - `seed` (int): The seed that determines the shuffle order.

    Returns (int): A new within-cohort index for the given item.

    Works just like `cohort_shuffle`, but applies the same operations in
    the opposite order, so that:

    ```py
    seed = 478273827
    y = cohort_shuffle(x, 100, seed)
    x == rev_cohort_shuffle(y, 100, seed)
    ```
    """
    r = inner
    seed = seed ^ cohort_size
    r = rev_cohort_spin(r, cohort_size, seed + 19) # prime
    r = cohort_upend(r, cohort_size, seed + 794641) # prime
    r = cohort_flop(r, cohort_size, seed + 193) # prime
    r = rev_cohort_interleave(r, cohort_size)
    r = rev_cohort_spread(r, cohort_size, seed + 881) # prime
    r = rev_cohort_mix(r, cohort_size, seed + 733) # prime
    r = rev_cohort_fold(r, cohort_size, seed + 211) # prime
    r = cohort_flop(r, cohort_size, seed + 53) # prime
    r = rev_cohort_interleave(r, cohort_size)
    r = rev_cohort_fold(r, cohort_size, seed + 839) # prime
    r = cohort_upend(r, cohort_size, seed + 47) # prime
    r = rev_cohort_spin(r, cohort_size, seed + 1987) # prime
    r = rev_cohort_interleave(r, cohort_size)
    r = rev_cohort_mix(r, cohort_size, seed + 2897) # prime
    r = rev_cohort_spread(r, cohort_size, seed + 457) # prime
    return r


# Distribution
# ------------

def distribution_spilt_point(
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Implements common distribution functionality: given a total item
    count, a segment count and per-segment capacity, and a roughness
    value and seed, computes the split point for the items as well as
    the halfway index for the segments.
    """

    # how the segments are divided:
    first_half = n_segments // 2

    # compute min/max split points according to roughness:
    nat = math.floor(total * (first_half / n_segments) )
    split_min = math.floor(nat - nat * roughness)
    split_max = math.floor(nat + (total - nat) * roughness)

    # adjust for capacity limits:
    if (total - split_min) > segment_capacity * (n_segments - first_half):
        split_min = total - (segment_capacity * (n_segments - first_half))

    if split_max > segment_capacity * first_half:
        split_max = segment_capacity * first_half

    # compute a random split point:
    split = nat
    if split_min >= split_max:
        split = split_min
    else:
        split = split_min + (
            rng.prng(total ^ rng.prng(seed, seed), seed)
          % (split_max - split_min)
        )

    return [split, first_half]


def distribution_portion(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Parameters:

    - `segment` (int): Which segment of a distribution we are
      interested in.
    - `total` (int): The total number of items being distributed.
    - `n_segments` (int): The number of segments to distribute items
      among.
    - `segment_capacity` (int): The capacity of each segment.
    - `roughness` (float in [0,1]): How rough the distribution should
      be.
    - `seed` (int): The seed that determines a specific distribution.

    Returns (int): the number of items from a distribution that end up
    in the given segment.

    Given that `total` items are to be distributed evenly among
    `n_segment` segments each with at most `segment_capacity` items and
    we're in segment `segment` of those, computes how many items are in
    this segment. The `roughness` argument must be a number between 0
    and 1 indicating how even the distribution is: 0 indicates a
    perfectly even distribution, while 1 indicates a perfectly random
    distribution. Does work proportional to the log of the number of
    segments.

    Note that segment_capacity * n_segments should be > total.
    """

    # base case
    if n_segments == 1:
        return total

    # compute split point:
    split, first_half = distribution_spilt_point(
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )

    # call ourselves recursively:
    if segment < first_half:
        return distribution_portion(
            segment,
            split,
            first_half,
            segment_capacity,
            roughness,
            seed
        )
    else:
        return distribution_portion(
            segment - first_half,
            total - split,
            n_segments - first_half,
            segment_capacity,
            roughness,
            seed
        )


def distribution_prior_sum(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Parameters:

    - `segment` (int): Which segment of a distribution we are
      interested in.
    - `total` (int): The total number of items being distributed.
    - `n_segments` (int): The number of segments to distribute items
      among.
    - `segment_capacity` (int): The capacity of each segment.
    - `roughness` (float in [0,1]): How rough the distribution should
      be.
    - `seed` (int): The seed that determines a specific distribution.

    Returns (int): the cumulative number of items from a distribution
    that are distributed before the given segment.

    Does similar math to the `distribution_portion` function above, but
    instead of returning the number of items in the given segment, it
    returns the number of items in all segments before the given
    segment. Only does work proportional to the log of the number of
    segments.
    """

    # base case
    if n_segments == 1:
        return 0 # nothing prior

    # compute split point:
    split, first_half = distribution_spilt_point(
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )

    # call ourselves recursively:
    if segment < first_half:
        return distribution_prior_sum(
            segment, # segment
            split, # total
            first_half, # n_segments
            segment_capacity, # capacity
            roughness, # roughness
            seed # seed
        )
    else:
        return split + distribution_prior_sum(
            segment - first_half,
            total - split,
            n_segments - first_half,
            segment_capacity,
            roughness,
            seed
        )


def distribution_segment(
    index,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Parameters:

    - `index` (int): Which item are we asking about.
    - `total` (int): The total number of items being distributed.
    - `n_segments` (int): The number of segments to distribute items
      among.
    - `segment_capacity` (int): The capacity of each segment.
    - `roughness` (float in [0,1]): How rough the distribution should
      be.
    - `seed` (int): The seed that determines a specific distribution.

    Returns (int): the index of the segment that the item of interest is
    distributed into.

    Computes the segment number in which a certain item appears (one of
    the `total` items distributed between segments; see
    `distribution_portion` above). Requires work proportional to the log
    of the number of segments.

    Note that the index should be between 0 and `total - 1`.
    """

    # base case
    if n_segments == 1:
        return 0 # we are in the only segment there is

    # compute split point:
    split, first_half = distribution_spilt_point(
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )

    # call ourselves recursively:
    if index < split:
        return distribution_segment(
            index,
            split,
            first_half,
            segment_capacity,
            roughness,
            seed
        )
    else:
        return first_half + distribution_segment(
            index - split,
            total - split,
            n_segments - first_half,
            segment_capacity,
            roughness,
            seed
        )


def max_smaller(value, sumtable):
    """
    Uses binary search to find and return the index of the largest sum in
    the given sumtable that's smaller than the given value. Works in time
    proportional to the logarithm of the sumtable size. Returns -1 if
    there is no entry in the sumtable smaller than the given value.

    TODO: more sumtable functions in Python?
    """
    fr = 0
    to = len(sumtable)
    where = 0
    while to - fr > 2:
        where = (to - fr) // 2
        if sumtable[where] >= value:
            to = where
        else:
            fr = where

    if to - fr == 1 and sumtable[fr] < value:
        return fr
    else:
        for i in range(fr, to - 1):
            if sumtable[i] < value and sumtable[i + 1] >= value:
                return i

    # no entry is smaller than the given value:
    return -1
