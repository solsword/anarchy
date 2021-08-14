"""
A module for incremental chaos. Includes reversible & incremental shuffle
& distribution operations, as well as lower-level reversible
pseudo-random (i.e., chaotic) number generation. Includes some
conveniences like pseudo-normally or exponentially-distributed chaotic
numbers.
"""

# Import version variable
from ._version import __version__ # noqa F401

# rng and cohort modules are available by default
from . import rng, cohort # noqa F401
