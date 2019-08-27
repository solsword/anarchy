"""
Empirically tests the period of the prng function with several seeds.
"""

import sys

import anarchy


seed = 9823098
starting_value = 0

seen = set()
rng = starting_value
period = 0
while rng not in seen:
  seen.add(rng)
  rng = anarchy.prng(rng, seed)
  period += 1
  if period % 1000000 == 0:
    print('.', end="")
    sys.stdout.flush()
print(
  "Seed {} starting from {}: period is {}".format(seed, starting_value, period)
)
