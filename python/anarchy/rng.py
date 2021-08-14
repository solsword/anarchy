"""
Core pseud-random number operations which provide reversibility.

rng.py
"""

import math


ID_BITS = 64
"""
The number of bits in an "ID" which is what most functions require as
seeds and/or return. We usually treat these as unsigned integers.
"""

ID_BYTES = ID_BITS >> 3
"""
The number of bytes in an "ID" value.
"""

ID_MASK = (1 << ID_BITS) - 1
"""
An "ID" value which is all-ones, to serve as a mask for preventing
overflow.
"""


def posmod(x, y):
    """
    Modulus that's always positive (x % y) as long as y is positive.

    Note: In python, the built-in % operator works this way, which is
    what this function uses. It is retained for cross-language
    compatibility, but in general the native operator should be preferred
    as it's faster.
    """
    return x % y


def hash_string(s):
    """
    A string hashing function.

    See: [StackOverflow answer on hashing strings](https://stackoverflow.com/questions/7616461/generate-a-hash-from-string-in-javascript-jquery).
    """ # noqa E501
    hash = 0
    if len(s) == 0:
        return hash

    for i, ch in enumerate(s):
        o = ord(ch)
        hash = ((hash << 5) - hash) + o
        hash &= ID_MASK

    return hash


def mask(bits):
    """
    Creates a mask with the given number of 1 bits.
    """
    return (1 << bits) - 1


def byte_mask(n):
    """
    Returns a mask that covers just the nth byte (zero-indexed), starting
    from the least-significant digits.
    """
    return (1 << (8 * n)) * 0xff


# Unit PRNG operations
#---------------------

def swirl(x, distance):
    """
    Circular bit shift. Distance is capped at 3/4 of ID_BITS.

    Inverse is `rev_swirl`.
    """
    distance %= (3 * ID_BITS) // 4
    m = mask(distance)
    fall_off = x & m
    shift_by = (ID_BITS - distance)
    return (
        (x >> distance)
      | (fall_off << shift_by)
    ) & ID_MASK


def rev_swirl(x, distance):
    """
    Inverse of `swirl`.
    """
    distance %= (3 * ID_BITS) // 4
    m = mask(distance)
    m = mask(distance)
    fall_off = x & (m << (ID_BITS - distance))
    shift_by = (ID_BITS - distance)
    return (
        (x << distance)
      | (fall_off >> shift_by)
    ) & ID_MASK


def fold(x, where):
    """
    Folds lower bits into upper bits using xor. 'where' is restricted to
    fall between 1/4 and 1/2 of ID_BITS.

    Fold is its own inverse.
    """
    quarter = ID_BITS // 4
    where = (where % quarter) + quarter
    m = mask(where)
    lower = x & m
    shift_by = ID_BITS - where
    return (x ^ (lower << shift_by)) & ID_MASK


FLOP_MASK = 0xf0f0f0f0f0f0f0f0
"""
A bit-mask used for the `flop` operation.
"""


def flop(x):
    """
    Flops each 1/2 byte with the adjacent 1/2 byte.

    Flop is its own inverse.
    """
    left = x & FLOP_MASK
    right = x & ~FLOP_MASK
    return ((right << 4) | (left >> 4)) & ID_MASK


def scramble(x):
    """
    Implements a reversible linear-feedback-shift-register-like operation.

    Inverse is `rev_scramble`.
    """
    trigger = x & 0x80200003
    r = swirl(x, 1)
    r ^= 0x03040610 * (not not trigger) # pseudo-if

    return r & ID_MASK


def rev_scramble(x):
    """
    Inverse of `scramble`.
    """
    pr = rev_swirl(x, 1)
    trigger = pr & 0x80200003
    if trigger:
        # pr ^= rev_swirl(0x03040610, 1)
        pr ^= 0x06080c20

    return pr & ID_MASK


# PRNG functions
#---------------

def scramble_seed(s):
    """
    Scrambles a seed value to help separate RNG sequences generated from
    sequential seeds.
    """

    s = s & ID_MASK
    s = ((s + 1) * (3 + (s % 23))) & ID_MASK
    s = fold(s, 11) & ID_MASK # prime
    s = scramble(s) & ID_MASK
    s = swirl(s, s + 23) & ID_MASK # prime
    s = scramble(s) & ID_MASK
    s ^= (s % 153) * scramble(s)
    s = s & ID_MASK

    return s


def prng(x, seed):
    """
    Parameters:

    - `x` (int): The current random number (next one is returned).
    - `seed` (int): The seed; each seed will produce a distinct sequence
      of numbers.

    Returns (int): The next random number in the sequence for the given
    seed.

    A reversible pseudo-random number generator. Uses low-level
    reversible functions to scramble a number and produce an
    hard-to-predict result. The `rev_prng` function is the inverse of
    this function.
    """

    # seed scrambling:
    seed = scramble_seed(seed)

    # value scrambling:
    x ^= seed
    x = fold(x, seed + 17) # prime
    x = flop(x)
    x = swirl(x, seed + 37) # prime
    x = fold(x, seed + 89) # prime
    x = swirl(x, seed + 107) # prime
    x = scramble(x)
    return x & ID_MASK


def rev_prng(x, seed):
    """
    Parameters:

    - `x` (int): the current random number (previous one is returned).
    - `seed` (int): The seed; each seed will produce a distinct
    sequence of numbers.

    Returns (int): The previous random number in the sequence for the
    given seed.

    The inverse of `prng` (see above). Returns the previous number in a
    pseudo-random sequence, so that:

    ```py
    prng(rev_prng(n, seed), seed) == n
    rev_prng(prng(n, seed), seed) == n
    ```
    """

    # seed scrambling:
    seed = scramble_seed(seed)

    # value unscrambling:
    x = rev_scramble(x)
    x = rev_swirl(x, seed + 107) # prime
    x = fold(x, seed + 89) # prime
    x = rev_swirl(x, seed + 37) # prime
    x = flop(x)
    x = fold(x, seed + 17) # prime
    x ^= seed
    return x & ID_MASK


def lfsr(x):
    """
    Parameters:

    - `x` (int): The current random number.

    Returns (int): The next random number in a pseudo-random sequence.

    Implements a max-cycle-length 64-bit linear-feedback-shift-register.
    See: [the Wikipedia article on Linear Feedback Shift
    Registers](https://en.wikipedia.org/wiki/Linear-feedback_shift_register).
    Note that this is NOT reversible!

    See also: [a PDF table of max-length LFSR
    parameters](http://courses.cse.tamu.edu/walker/csce680/lfsr_table.pdf).
    """
    lsb = x & 1
    r = x >> 1
    # pseudo-if
    r ^= lsb * 0xe800000000000000 # 64, 63, 61, 60

    return r & ID_MASK


# Sampling functions
#-------------------

def uniform(seed):
    """
    Parameter (int): `seed`--The seed that determines the result (could
      be a result from `prng` or `lfsr`)

    Returns (float): A pseudo-random number between 0 (inclusive) and 1
      (exclusive).
    """
    # Note: without this extra scrambling, udist on sequential seeds is
    # *terrible* for seeds below ~1000000.
    sc = scramble_seed(seed)
    sc = prng(prng(sc, sc), seed)
    return (sc % 9223372036854775783) / 9223372036854775783 # prime near 2^63


def normalish(seed):
    """
    Parameter (int): `seed`--The seed that determines the result.

    Returns (float): A pseudo-random number between 0 (inclusive) and 1
      (exclusive).

    The result is created by averaging three results from `uniform`, and
    its distribution has a normal-like shape centered at 0.5.
    """
    t = 0
    for i in range(3):
        t += uniform(seed + 9182793183 * i)
    return t / 3


def flip(p, seed):
    """
    Parameters:

    - `p` (float): The probability of returning True.
    - `seed` (int): The seed value that determines the result.

    Returns (True or False): a decision value.

    Flips a biased coin with probability `p` of being true. Using the
    same seed always gives the same result, but over many seeds the given
    probability is adhered to.
    """
    return uniform(prng(seed, seed)) < p


def integer(seed, start, end):
    """
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
    """
    return math.floor(uniform(seed) * (end - start)) + start


def exponential(seed, shape):
    """
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
    """ # noqa E501
    # Note: mathematically since u is on [0, 1) and not (0, 1], we can't
    # skip the 1- step, since when u = 0, that would be asking for the
    # log of 0. With 1-, the log of 0 would happen when u=1, but our
    # domain is [0, 1) which excludes 1.
    return -math.log(1 - uniform(seed)) / shape


def truncated_exponential(seed, shape):
    """
    Parameters:

    - `seed` (int)--The seed that determines the result.
    - `shape` (float)--The lambda shape parameter for the exponential
      distribution. Values between 0.5 and 1.5 are typical, with higher
      values biasing the distribution more towards smaller results.

    Returns (float) A number with a truncated exponential distribution
    on [0, 1].

    Generates a number from a truncated exponential distribution on [0,
    1] with the given lambda shape parameter. See reference material for
    `exponential`.
    """
    e = exponential(seed, shape)
    return e - math.floor(e)
