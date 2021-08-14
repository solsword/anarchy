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
