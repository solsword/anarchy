import pydoc
import sys

import anarchy

def unindent(docstr):
  """
  Strips indentation from a docstring that's in this format.
  """
  indent = 0
  for i, c in enumerate(docstr):
    if i == 0 and c == '\n':
      continue
    if c != ' ':
      break
    indent += 1
  lines = docstr.split('\n')
  result = ''
  for line in lines:
    if line == '':
      continue
    if line[:indent].strip() != '':
      print("Warning: non-indented line:\n{}".format(line), file=sys.stderr)
    result += line[indent:] + '\n'

  return result

INTRO = """
`anarchy` is a library for generating random numbers in an incremental and
reversible fashion. It also has routines for incremental shuffling and
distribution of items. The goal is to give a high level of control to designers
of PCG systems while maintaining incremental operation as is necessary in
incrementally-generated open-world situations.

However, it can also be used in simpler situations as it provides a variety of
built-in convenience functions like pseudo-gaussian and
exponentially-distributed random numbers (these are not reversible).

## Replacing `random()`

The incremental shuffling algorithm can be used as a replacement for a standard
random number generator in cases where you want to guarantee a global
distribution of items and are currently using independent random number checks
to control item distribution. For example, if you have code that looks like
this:

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

You have no guarantees about exactly how often rare/uncommon items will be, and
some players will get lucky or unlucky. Instead, even if you don't know the
number of roll_item calls, with `anarchy` you can do this:

```python
N = 0
seed = 472389223

def roll_item():
  global N, seed
  r = anarchy.cohort_shuffle(N, 100, seed + N//100)
  if r < 1:
    return "Rare"
  elif r < 6:
    return "Uncommon"
  else:
    return "Common"
```

In this code there are two extra variables that have to be managed in some way,
but the benefit is that for every 100 calls to the function, "Rare" will be
returned exactly once, and "Uncommon" will be returned exactly 5 times. Each
group of 100 calls will still have a different ordering of the items, because
it uses a different seed.

There are many other potential applications of reversible/incremental
shuffling; this is one of the most direct.

## Core Functions

(Note: these are the most useful functions available but there are others, like
the individual reversible operations used for shuffling or random number
generation. Those functions are documented in the source code.)

"""

PRNG_FUNCTIONS = [
  anarchy.prng,
  anarchy.rev_prng,
  anarchy.udist,
  anarchy.pgdist,
  anarchy.flip,
  anarchy.idist,
  anarchy.expdist,
]

COHORT_FUNCTIONS = [
  anarchy.cohort,
  anarchy.cohort_inner,
  anarchy.cohort_outer,
  anarchy.cohort_shuffle,
  anarchy.rev_cohort_shuffle,
  anarchy.distribution_portion,
  anarchy.distribution_prior_sum,
  anarchy.distribution_segment,
]

with open("doc.mkd", 'w') as fout:
  fout.write("""\
---
title: Anarchy Library Documentation
author: Peter Mawhorter (pmawhorter@gmail.com)
...
# Anarchy Library Documentation
""")
  #fout.write("# `anarchy` Library Documentation\n")
  fout.write(INTRO)
  fout.write(
    "### Single Number Functions\n\nDeal with individual random numbers.\n\n"
  )
  for f in PRNG_FUNCTIONS:
    fout.write("- [`{}`](#{})\n".format(f.__name__, f.__name__))
  for f in PRNG_FUNCTIONS:
    fout.write("\n### `{}`\n\n".format(f.__name__, f.__name__))
    fout.write(unindent(f.__doc__) + '\n')

  fout.write(
    "### Cohort Functions\n\nDeal similarly with numbers from a range.\n\n"
  )
  for f in COHORT_FUNCTIONS:
    fout.write("- [`{}`](#{})\n".format(f.__name__, f.__name__))
  for f in COHORT_FUNCTIONS:
    fout.write("\n### `{}`\n\n".format(f.__name__, f.__name__))
    fout.write(unindent(f.__doc__) + '\n')
