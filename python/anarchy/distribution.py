"""
Reversible & incremental distribution across segments.

distribution.py

Segments are conceptually finite-capacity parts of a larger region, which
has some pre-determined total number of segments. If we have some total
number of items, we could easily divide them evenly between the segments,
but what if we want to apportion them randomly between the segments? The
functions in this file all require a total count, a segment count, and a
segment capacity. Given these things they can:

1. Compute a 'split point' indicating how many of the total items fall
    into the first half of the segments (`distribution_spilt_point`).
2. Compute, for a given segment, how many items are distributed before
    that segment, and how many items are in that segment
    (`distribution_items`, or individually `distribution_prior_sum` and
    `distribution_portion`).
3. Compute, for a given item out of the total items, which segment it
    falls into (`distribution_segment`).

We do each of these things in logarithmic time based on the number of
segments, without needing to compute the full distribution of items to
segments ahead of time or store in in memory. The strategy is to
recursively compute split points only for the half of the distribution
that we fall into.

The 'roughness' parameter for these functions can be used to control how
even (0) vs. rough (1) the distribution of items over segments is. At
zero roughness, exactly 50% of the items will be in the first half of the
segments. At one roughness, anywhere from 0% to 100% of the items could
be placed into the first half of the segments.
"""

import math

from . import rng

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


def distribution_items(
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

    Returns (2-tuple of ints): the number of items from a distribution
    that come before the start of the given segment, and the number of
    items in that segment.

    Note: Consider constructing a sumtable and using `sumtable_items`
    instead for faster results at the cost of a bit of memory (see
    `sumtable_for_distribution`).

    Given that `total` items are to be distributed evenly among
    `n_segment` segments each with at most `segment_capacity` items and
    we're in segment `segment` of those, computes how many items come
    before this segment and how many are in it. The `roughness` argument
    must be a number between 0 and 1 indicating how even the distribution
    is: 0 indicates a perfectly even distribution, while 1 indicates a
    perfectly random distribution. Does work proportional to the log of
    the number of segments.

    Note that segment_capacity * n_segments should be >= total.
    """

    # base case
    if n_segments == 1:
        return 0, total

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
        return distribution_items(
            segment,
            split,
            first_half,
            segment_capacity,
            roughness,
            seed
        )
    else:
        additional_before, portion = distribution_items(
            segment - first_half,
            total - split,
            n_segments - first_half,
            segment_capacity,
            roughness,
            seed
        )
        return split + additional_before, portion


def distribution_portion(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Works like `distribution_items` but only returns the count of items in
    the indicated segment. Preserved for backwards compatibility with
    versions before `distribution_items` was added.
    """

    return distribution_items(
        segment,
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )[1]


def distribution_prior_sum(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Works like `distribution_items` but only returns the number of items
    that come before the indicated segment. Preserved for backwards
    compatibility with versions before `distribution_items` was added.
    """

    return distribution_items(
        segment,
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )[0]


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
    - `roughness` (float in [0,1]): How rough the distribution should be.
    - `seed` (int): The seed that determines a specific distribution.

    Returns (int): the index of the segment that the item of interest is
    distributed into.

    Note: Consider constructing a sumtable and using `sumtable_segment`
    instead for faster results at the cost of a bit of memory (see
    `sumtable_for_distribution`).

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


def sumtable_for_distribution(
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Parameters:

    - `total` (int): The total number of items being distributed.
    - `n_segments` (int): The number of segments to distribute items
        among.
    - `segment_capacity` (int): The capacity of each segment.
    - `roughness` (float in [0,1]): How rough the distribution should be.
    - `seed` (int): The seed that determines a specific distribution.

    Takes distribution parameters and returns a list containing the
    cumulative sum of items in each segment. The list contains
    `n_segments` entries, and each entry holds the sum of the items from
    the `total` assigned to that segment plus all earlier segments. This
    means that each entry is greater than or equal to the entry to its
    left (and they're only equal if zero items are assigned to that
    segment). The last entry will be equal to the total.

    `n_segments` times `segment_capacity` must be >= `total` otherwise a
    `ValueError` will be raised.
    """

    if n_segments * segment_capacity < total:
        raise ValueError(
            f"Cannot distribute {total} item(s) across {n_segments}"
            f" segments of size {segment_capacity}: There's not enough"
            f" space for them all to fit."
        )

    table = [0] * n_segments
    fill_sumtable_for_distribution(
        table,
        0,
        0,
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )
    return table


def fill_sumtable_for_distribution(
    table,
    start,
    prior,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
):
    """
    Parameters are the same as for `sumtable_for_distribution` but with:

    - table (list of ints): The table to fill in. Its length must be at
        least `start` + `n_segments` but its values will be overwritten
        (at least between `start` and `start + `n_segments`).
    - start (int): The index at which to start overwriting values.
    - prior (int): The number of items distributed prior to the start
        index.

    Recursively fills in the sumtable based on the distribution
    parameters given (see `sumtable_for_distribution`). Total work done
    is `n_segments` times log(`n_segments`).

    Note that using a sumtable instead of using the distribution
    functions with the same parameters over and over again represents a
    space/time tradeoff. Most operations on a sumtable are O(1), where
    equivalent distribution functions need O(log(n)) time. The sumtable
    needs to store one integer per segment. The use of distribution
    functions is recommended primarily in situations where the algorithm
    structure makes it inconvenient to store the sumtable in an
    accessible way, so sharing parameters is easier.
    """
    if n_segments == 0:  # base case: nothing to modify
        assert total == 0
        return
    elif n_segments == 1:  # base case: fill in one entry
        table[start] = prior + total
    else:  # recurse into each half of the table
        sub_prior, first_half = distribution_spilt_point(
            total,
            n_segments,
            segment_capacity,
            roughness,
            seed
        )
        # Fill in the first half
        fill_sumtable_for_distribution(
            table,
            start,
            prior,
            sub_prior,
            first_half,
            segment_capacity,
            roughness,
            seed
        )
        # Fill in the second half
        fill_sumtable_for_distribution(
            table,
            start + first_half,
            prior + sub_prior,
            total - sub_prior,
            n_segments - first_half,
            segment_capacity,
            roughness,
            seed
        )


def sumtable_from_portions(portions):
    """
    Parameters:

    - `portions` (list of integers): A list where each entry holds the
        portion of some total that's assigned to a segment at that index.

    Returns a sumtable for the same distribution, where entries are the
    cumulative sum of all portions up to an index, rather than just the
    portion at that index. Use functions like `sumtable_total`,
    `sumtable_segments`, `sumtable_portion`, and `sumtable_segment` to
    get information out of the sumtable efficiently.
    """
    sofar = 0
    result = []
    for p in portions:
        sofar += p
        result.append(sofar)

    return result


def sumtable_total(sumtable):
    """
    Parameters:

    - `sumtable` (list of integers): The table of cumulative sums
        describing a distribution of items across segments.

    Returns the total number of items across all segments in a sumtable,
    which is just the last entry in that table.
    """
    return sumtable[-1]


def sumtable_segments(sumtable):
    """
    Parameters:

    - `sumtable` (list of integers): The table of cumulative sums
        describing a distribution of items across segments.

    Returns the number of segments in the given sumtable, which is just
    its length.
    """
    return len(sumtable)


def sumtable_portion(sumtable, i):
    """
    Parameters:

    - `sumtable` (list of integers): The table of cumulative sums
        describing a distribution of items across segments.
    - `i` (int): The index to compute the portion for.

    Returns the portion of the total items which is distributed into the
    segment with the given index (starting from 0). Just needs to
    subtract the entry to the left to figure this out.

    Treats negative indices as indexing back from the end of the
    sumtable.
    """
    # Convert negative indices to positive ones
    if i < 0:
        i += len(sumtable)
        # Error if the index was too far negative
        if i < 0:
            raise IndexError(
                f"Invalid index {i - len(sumtable)} for table with"
                f" {len(sumtable)} entries (too negative)."
            )

    # First entry has nothing to subtract
    if i == 0:
        return sumtable[0]
    elif i < len(sumtable):
        # In-bounds entries just subtract sums to get value
        return sumtable[i] - sumtable[i - 1]
    else:
        # Out-of-bounds past the end
        raise IndexError(
            f"Invalid index {i} for table with {len(sumtable)} entries"
            f" (too large)."
        )


def sumtable_items(sumtable, i):
    """
    Parameters:

    - `sumtable` (list of integers): The table of cumulative sums
        describing a distribution of items across segments.
    - `i` (int): The index to compute the items for.

    Returns a pair containing the number of items prior to the specified
    segment and then the number of items in that segment, like
    `distribution_items`, but for a sumtable.
    """
    portion = sumtable_portion(sumtable, i)  # index error here if bad i

    # Read prior sum directly from adjacent table entry
    if i == 0 or i == -len(sumtable):
        prior = 0
    else:
        prior = sumtable[i - 1]

    return (prior, portion)


def sumtable_segment(sumtable, item):
    """
    Parameters:

    - `sumtable` (list of integers): The table of cumulative sums
        describing a distribution of items across segments.
    - `item` (int): The item index out of the total items that we want to
        figure out the segment for (starting from 0 for the first item).

    Returns (int): The segment index that the given item falls into based
    on the sumtable.

    Raises a `ValueError` if the sumtable is empty, and raises an
    `IndexError` if the item index is too large for the sumtable or if
    it's negative.

    Uses binary search to find the smallest sum in the given sumtable
    that's larger than the given value breaking ties towards earlier
    entries. Works in time proportional to the logarithm of the sumtable
    size.
    """
    if len(sumtable) == 0:
        raise ValueError("Empty sumtable in sumtable_segment.")
    elif item < 0:
        raise IndexError(
            f"Negative item index {item} not permitted in"
            f" sumtable_segment."
        )
    elif item >= sumtable[-1]:
        raise IndexError(
            f"Item index {item} is too large in sumtable_segment. The"
            f" sumtable only contains {sumtable[-1]} items."
        )

    fr = 0  # inclusive
    to = len(sumtable) - 1  # inclusive
    where = None
    # Keep splitting until we narrow it down to a single position (or none)
    while to - fr > 0:
        where = fr + (to - fr) // 2  # halfway between
        if sumtable[where] <= item:  # definitely not here; it's farther
            fr = where + 1
        else:  # could be here
            to = where

    if to - fr < 0:
        # shouldn't be possible in a valid sumtable given the checks
        # above...
        frspec = f"{fr} (value {sumtable[fr]})"
        tospec = f"{to} (value {sumtable[to]})"
        raise RuntimeError(
            f"Item index {item} not found in sumtable. Must be at least"
            f" at index {frspec} but may not be past index {tospec}."
        )

    assert to == fr
    # Check the single remaining entry & return
    if sumtable[fr] <= item:
        # Shouldn't be possible with a valid sumtable given checks above
        # that the index is in range of the total
        raise RuntimeError(
            f"Item index {item} not found in sumtable. Must be at index"
            f" {fr} (value {sumtable[fr]}) but that value is too small."
        )
    else:
        return fr
