---
title: Anarchy Library Documentation
author: Peter Mawhorter (pmawhorter@gmail.com)
...
## Overview

`anarchy` is a package for managed & beautiful chaos, designed with
incremental procedural content generation in mind. It includes
incremental & reversible random numbers, a selection of distribution
functions for random values, and cohort operations that can be applied
incrementally to groups along an indefinite 1-dimensional space.

The goal is to give a high level of control to designers of PCG systems
while maintaining incremental operation.

Coming soon: fractal coordinates.


## Versions

There are implementations of at least the core functionality available in
C, C#, Javascript, and Python; this documentation applies most closely to
the Python implementation, and it is drawn from that code. Each different
language implementation has its own idiosyncrasies, but the higher-level
things, like number and meaning of parameters, are the same for the core
functions.

TODO: Links to versions...

Note: the anarchy Python package uses 64-bit integers, for compatibility
with the C version of the library and for a larger output space.
Technical limitations with JavaScript mean that the JS version of the
library uses 32-bit integers and will therefore give different results.


## Dependencies

The python version requires Python 3; tests use `pytest` and require
Python >=3.6.


## Example Application

The incremental shuffling algorithm can be used as a replacement for a
standard random number generator in cases where you want to guarantee a
global distribution of items and are currently using independent random
number checks to control item distribution. For example, if you have code
that looks like this...

```python
def roll_item():
  r = random.random()
  if r < 0.01: # 1% chance for Rare item
    return "Rare"
  elif r < 0.06: # 5% chance for Uncommon item
    return "Uncommon"
  else:
    return "Common"
```

...you have no guarantees about exactly how often rare/uncommon items
will be, and some players will get lucky or unlucky. Instead, even if you
don't know the number of `roll_item` calls, with `anarchy` you can do
this:

```python
N = 0
seed = 472389223

def roll_item():
  global N, seed
  r = anarchy.cohort_shuffle(N, 100, seed + N//100)
  N += 1
  if r < 1:
    return "Rare"
  elif r < 6:
    return "Uncommon"
  else:
    return "Common"
```

In this code there are two extra variables that have to be managed in
some way, but the benefit is that for every 100 calls to the function,
"Rare" will be returned exactly once, and "Uncommon" will be returned
exactly 5 times. Each group of 100 calls will still have a different
ordering of the items, because it uses a different seed.

Here's an image illustrating the differences between these two
approaches: in the top half, results are generated using independent
random numbers, while the bottom half uses anarchy's cohort shuffling to
guarantee one red cell and 5 blue cells per 10×10 white square..

![100 10x10 squares in which blue and/or red pixels are placed. In the top 50 squares, each pixel has an independent 1% chance of being red and 5% chance of being blue, and as a result, some of the white squares have zero red pixels, or have more red pixels than blue pixels, or have many more than 5 blue pixels or 1 red pixel. In the bottom half, anarchy cohort shuffling is used, so while the positions of the colored pixels are different in each block, each block contains exactly 1 red pixel and exactly 5 blue pixels.](demos/rng_shuf_compare.png){.pixels}\ 

There are many other potential applications of reversible/incremental
shuffling; this is one of the most direct.


## Contents

### Single Number Functions

Deal with individual random numbers.

- [`rng.prng`](#rng.prng)
- [`rng.rev_prng`](#rng.rev_prng)
- [`rng.uniform`](#rng.uniform)
- [`rng.normalish`](#rng.normalish)
- [`rng.flip`](#rng.flip)
- [`rng.integer`](#rng.integer)
- [`rng.exponential`](#rng.exponential)
- [`rng.truncated_exponential`](#rng.truncated_exponential)


### Cohort Functions

Deal consistently with numbers in groups.

- [`cohort.cohort`](#cohort.cohort)
- [`cohort.cohort_inner`](#cohort.cohort_inner)
- [`cohort.cohort_outer`](#cohort.cohort_outer)
- [`cohort.cohort_shuffle`](#cohort.cohort_shuffle)
- [`cohort.rev_cohort_shuffle`](#cohort.rev_cohort_shuffle)
- [`cohort.distribution_portion`](#cohort.distribution_portion)
- [`cohort.distribution_prior_sum`](#cohort.distribution_prior_sum)
- [`cohort.distribution_segment`](#cohort.distribution_segment)

## Functions


### `rng.prng`


Parameters:

- `x` (int): The current random number (next one is returned).
- `seed` (int): The seed; each seed will produce a distinct sequence
  of numbers.

Returns (int): The next random number in the sequence for the given
seed.

A reversible pseudo-random number generator. Uses low-level
reversible functions to scramble a number and produce an
hard-to-predict result. The [`rev_prng`](#rng.rev_prng) function is the inverse of
this function.



### `rng.rev_prng`


Parameters:

- `x` (int): the current random number (previous one is returned).
- `seed` (int): The seed; each seed will produce a distinct
sequence of numbers.

Returns (int): The previous random number in the sequence for the
given seed.

The inverse of [`prng`](#rng.prng) (see above). Returns the previous number in a
pseudo-random sequence, so that:

```py
prng(rev_prng(n, seed), seed) == n
rev_prng(prng(n, seed), seed) == n
```



### `rng.uniform`


Parameter (int): `seed`--The seed that determines the result (could
  be a result from [`prng`](#rng.prng) or `lfsr`)

Returns (float): A pseudo-random number between 0 (inclusive) and 1
  (exclusive).



### `rng.normalish`


Parameter (int): `seed`--The seed that determines the result.

Returns (float): A pseudo-random number between 0 (inclusive) and 1
  (exclusive).

The result is created by averaging three results from [`uniform`](#rng.uniform), and
its distribution has a normal-like shape centered at 0.5.



### `rng.flip`


Parameters:

- `p` (float): The probability of returning True.
- `seed` (int): The seed value that determines the result.

Returns (True or False): a decision value.

Flips a biased coin with probability `p` of being true. Using the
same seed always gives the same result, but over many seeds the given
probability is adhered to.



### `rng.integer`


Parameters:

  - `seed` (int): The seed value that determines the result.
  - `start` (int): The minimum possible output value.
  - `end` (int): One more than the maximum possible output value.

Returns (int): a pseudo-random integer between the start (inclusive)
and the end (exclusive).

Returns a number drawn evenly from the given integer range, including
the lower end but excluding the higher end (even if the lower end is
given second).

Distribution bias is about one part in (range/2^63).



### `rng.exponential`


Parameters:

- `seed` (int)--The seed that determines the result.
- `shape` (float):--The lambda shape parameter for the exponential
  distribution. Values between 0.5 and 1.5 are typical, with higher
  values biasing the distribution more towards smaller results.

Returns (float): A number with an exponential distribution on [0,∞).

Generates a number from an exponential distribution with the given
lambda shape parameter.

See: [this StackExchange answer on exponential
distribution](https://math.stackexchange.com/questions/28004/random-exponential-like-distribution)
and [the Wikipedia page for the exponential
distribution](https://en.wikipedia.org/wiki/Exponential_distribution)



### `rng.truncated_exponential`


Parameters:

- `seed` (int)--The seed that determines the result.
- `shape` (float)--The lambda shape parameter for the exponential
  distribution. Values between 0.5 and 1.5 are typical, with higher
  values biasing the distribution more towards smaller results.

Returns (float) A number with a truncated exponential distribution
on [0, 1].

Generates a number from a truncated exponential distribution on [0,
1] with the given lambda shape parameter. See reference material for
[`exponential`](#rng.exponential).



### `cohort.cohort`


Parameters:

- `outer` (int): An integer to be assigned to a cohort.
- `cohort_size` (int): The size of each cohort.

Returns (int): Which cohort the given outer item is assigned to.

Simply returns the outer index divided by the cohort size, rounded
down. Note that for extremely large integers, they will be truncated
to fit within 64 bits.



### `cohort.cohort_inner`


Parameters:

- `outer` (int): An integer to be assigned to a cohort.
- `cohort_size` (int): The size of each cohort.

Returns (int): Where within its cohort the given integer ends up.

Computes within-cohort index for the given outer index and cohorts of
the given size, which is just the index modulus the cohort size. Like
[`cohort`](#cohort.cohort), truncates to 64 bits.



### `cohort.cohort_outer`


Parameters:

- [`cohort`](#cohort.cohort) (int): Which cohort the item is in.
- `inner` (int): The index of the item in that cohort.
- `cohort_size` (int): The size of each cohort.

Returns (int): The outer index that would be mapped to this
within-cohort index.

This is the Inverse of `cohort_and_inner`: it computes the outer
index from a cohort number and inner index.



### `cohort.cohort_shuffle`


Parameters:

- `inner` (int): The index of an item in a cohort.
- `cohort_size` (int): The size of each cohort.
- `seed` (int): The seed that determines the shuffle order.

Returns (int): A new within-cohort index for the given item.

Implements an incremental shuffle, such that if you were to call
[`cohort_shuffle`](#cohort.cohort_shuffle) on each index in a cohort of a given size, each of
the items would end up in a different new spot within that cohort.
The shuffle is reversible using the [`rev_cohort_shuffle`](#cohort.rev_cohort_shuffle) function.

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



### `cohort.rev_cohort_shuffle`


Parameters:

- `inner` (int): The index of an item in a cohort.
- `cohort_size` (int): The size of each cohort.
- `seed` (int): The seed that determines the shuffle order.

Returns (int): A new within-cohort index for the given item.

Works just like [`cohort_shuffle`](#cohort.cohort_shuffle), but applies the same operations in
the opposite order, so that:

```py
seed = 478273827
y = cohort_shuffle(x, 100, seed)
x == rev_cohort_shuffle(y, 100, seed)
```



### `cohort.distribution_portion`


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



### `cohort.distribution_prior_sum`


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

Does similar math to the [`distribution_portion`](#cohort.distribution_portion) function above, but
instead of returning the number of items in the given segment, it
returns the number of items in all segments before the given
segment. Only does work proportional to the log of the number of
segments.



### `cohort.distribution_segment`


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
[`distribution_portion`](#cohort.distribution_portion) above). Requires work proportional to the log
of the number of segments.

Note that the index should be between 0 and `total - 1`.


