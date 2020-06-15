"""
test_anarchy.py
Testing module for anarchy reversible RNG library.
"""

import anarchy
import math

VALUE_TESTS = {
  "posmod": [
    [ anarchy.posmod(-1, 7), 6 ],
    [ anarchy.posmod(0, 11), 0 ],
    [ anarchy.posmod(3, 11), 3 ],
    [ anarchy.posmod(-13, 11), 9 ],
    [ anarchy.posmod(15, 11), 4 ],
    [ anarchy.posmod(115, 10), 5 ],
    [ anarchy.posmod(115, 100), 15 ],
  ],
  "mask": [
    [ anarchy.mask(0), 0 ],
    [ anarchy.mask(1), 1 ],
    [ anarchy.mask(2), 3 ],
    [ anarchy.mask(4), 15 ],
    [ anarchy.mask(10), 1023 ],
  ],
  "byte_mask": [
    [ anarchy.byte_mask(0), 255 ],
    [ anarchy.byte_mask(1), 65280 ],
    [ anarchy.byte_mask(2), 16711680 ],
  ],
  "swirl": [
    [ anarchy.swirl(2, 1), 1 ],
    [ anarchy.swirl(4, 1), 2 ],
    [ anarchy.swirl(8, 2), 2 ],
    [ anarchy.swirl(1, 1), 0x8000000000000000 ],
    [ anarchy.swirl(2, 2), 0x8000000000000000 ],
    [ anarchy.swirl(1, 2), 0x4000000000000000 ],
    [ anarchy.rev_swirl(1, 2), 4 ],
    [ anarchy.rev_swirl(1, 3), 8 ],
    [ anarchy.rev_swirl(2, 2), 8 ],
    [ anarchy.rev_swirl(0x8000000000000000, 1), 1 ],
    [ anarchy.rev_swirl(0x8000000000000000, 2), 2 ],
    [ anarchy.rev_swirl(0x8000000000000000, 3), 4 ],
    [ anarchy.rev_swirl(0x4000000000000000, 3), 2 ],
    [ anarchy.rev_swirl(0x0000000000101030, 1), 0x0000000000202060 ],
    [
      anarchy.rev_swirl(anarchy.swirl(1098301, 17), 17),
      1098301
    ],
  ],
  "fold": [
    # TODO: Verify these!
    [ anarchy.fold(22908, 7), 50375224738208124 ],
    [ anarchy.fold(18201, 18), 1280781512777680665 ],
    [ anarchy.fold(anarchy.fold(18201, 18), 18), 18201 ],
    [ anarchy.fold(anarchy.fold(89348393, 1289), 1289), 89348393 ],
  ],
  "flop": [
    # TODO: Verify these!
    [ anarchy.flop(0xf0f0f0f0), 0x0f0f0f0f ],
    [ anarchy.flop(0x0f0f0f0f), 0xf0f0f0f0 ],
    [ anarchy.flop(22908), 38343 ],
    [ anarchy.flop(18201), 29841 ],
    [ anarchy.flop(anarchy.flop(3892389)), 3892389 ],
    [ anarchy.flop(anarchy.flop(489248448)), 489248448 ],
  ],
  "scramble": [
    [
      anarchy.scramble(
        anarchy.rev_swirl(0x03040610 | 0x40004001, 1)
      ),
      0x40004001
    ],
    [
      anarchy.rev_scramble(0x40004001),
      anarchy.rev_swirl(0x03040610 | 0x40004001, 1)
    ],
    [ anarchy.rev_scramble(anarchy.scramble(17)), 17 ],
    [ anarchy.rev_scramble(anarchy.scramble(8493489)), 8493489 ],
    [ anarchy.scramble(anarchy.rev_scramble(8493489)), 8493489 ],
  ],
  "prng": [
    [ anarchy.prng(489348, 373891), 18107188676709054266 ],
    [ anarchy.rev_prng(18107188676709054266, 373891), 489348 ],
    [ anarchy.rev_prng(1766311112, 373891), 14566213110237565854 ],
    [ anarchy.prng(0, 0), 15132939213242511212 ],
    [ anarchy.rev_prng(15132939213242511212, 0), 0 ],
    [ anarchy.prng(anarchy.rev_prng(1782, 39823), 39823), 1782 ],
    [ anarchy.rev_prng(anarchy.prng(1782, 39823), 39823), 1782 ],
  ],
  "lfsr": [
    # TODO: Verify these!
    [ anarchy.lfsr(489348), 244674 ],
    [ anarchy.lfsr(1766932808), 883466404 ],
  ],
  "udist": [
    [ anarchy.udist(0), 0.842373086655968 ],
    [ anarchy.udist(8329801), 0.5186867891411134 ],
    [ anarchy.udist(58923), 0.9716616308000062 ],
  ],
  "pgdist": [
    [ anarchy.pgdist(0), 0.6184944203669203 ],
    [ anarchy.pgdist(8329801), 0.4333781836142186 ],
    [ anarchy.pgdist(58923), 0.6939341836590818 ],
  ],
  "idist": [
    [ anarchy.idist(0, 0, 1), 0 ],
    [ anarchy.idist(0, 3, 25), 21 ],
    [ anarchy.idist(58923, 3, 25), 24 ],
    [ anarchy.idist(58923, -2, -4), -4 ],
  ],
  "expdist": [
    [ anarchy.expdist(0, 0.5), 3.6950486923768895 ],
    [ anarchy.expdist(8329801, 0.5), 1.4624741095323832 ],
    [ anarchy.expdist(58923, 1.5), 2.375692393655211 ],
  ],
  "trexpdist": [
    [ anarchy.trexpdist(0, 0.5), 0.6950486923768895 ],
    [ anarchy.trexpdist(8329801, 1.5), 0.4874913698441277 ],
    [ anarchy.trexpdist(58923, 2.5), 0.4254154361931266 ],
  ],
  "cohorts": [
    [ anarchy.cohort(17, 3), 5 ],
    [ anarchy.cohort(10, 10), 1 ],
    [ anarchy.cohort(9, 10), 0 ],
    [ anarchy.cohort_inner(17, 3), 2 ],
    [ anarchy.cohort_inner(10, 10), 0 ],
    [ anarchy.cohort_inner(9, 10), 9 ],
    [ anarchy.cohort_and_inner(9, 10)[0], 0 ],
    [ anarchy.cohort_and_inner(9, 10)[1], 9 ],
    [ anarchy.cohort_outer(0, 3, 112), 3 ],
    [ anarchy.cohort_outer(1, 3, 112), 115 ],
    [ anarchy.cohort_outer(-1, 3, 112), 18446744073709551507 ],
  ],
  "cohort_interleave": [
    [ anarchy.cohort_interleave(3, 12), 6 ],
    [ anarchy.cohort_interleave(7, 12), 9 ],
  ],
  "cohort_spin": [
    [ anarchy.cohort_spin(0, 2, 1048239), 1 ],
    [ anarchy.rev_cohort_spin(1, 2, 1048239), 0 ],
  ],
  "cohort_mix": [
    [ anarchy.cohort_mix(0, 3, 0), 2 ],
    [ anarchy.cohort_mix(1, 3, 0), 1 ],
    [ anarchy.cohort_mix(2, 3, 0), 0 ],
    [ anarchy.rev_cohort_mix(2, 3, 0), 0 ],
    [ anarchy.rev_cohort_mix(1, 3, 0), 1 ],
    [ anarchy.rev_cohort_mix(0, 3, 0), 2 ],
  ],
  "cohort_fold": [
    [ anarchy.cohort_fold(0, 3, 0), 0 ],
    [ anarchy.cohort_fold(1, 3, 0), 2 ],
    [ anarchy.cohort_fold(2, 3, 0), 1 ],
    [ anarchy.rev_cohort_fold(0, 3, 0), 0 ],
    [ anarchy.rev_cohort_fold(1, 3, 0), 2 ],
    [ anarchy.rev_cohort_fold(2, 3, 0), 1 ],
  ],
  "cohort_shuffle": [
    [ anarchy.cohort_shuffle(0, 3, 17), 0 ],
    [ anarchy.cohort_shuffle(1, 3, 17), 2 ],
    [ anarchy.cohort_shuffle(2, 3, 17), 1 ],
    [ anarchy.rev_cohort_shuffle(0, 3, 17), 0 ],
    [ anarchy.rev_cohort_shuffle(2, 3, 17), 1 ],
    [ anarchy.rev_cohort_shuffle(1, 3, 17), 2 ],
  ],
}

TEST_VALUES = [
  0,
  1,
  3,
  17,
  48,
  64,
  1029,
  8510938,
  1928301928,
  1 << 31
]

SEQ_SEEDS = list(range(10000))

N_SAMPLES = 10000
N_CDF_BUCKETS = 100

EXEC_TESTS = {}

def test(fn):
  global EXEC_TESTS
  EXEC_TESTS[fn.__name__] = fn

@test
def unit_ops():
  result = 0
  messages = []
  seed = 129873918231
  for o in (0, 12, 24, 36, 48):
    print("\nOffset:", o)
    for i in range(1 << 12):
      if i % (1 << 8) == (1 << 8) - 1:
        print("{:0.0f}% done".format(100 * i/(1 << 12)), end='\r')
      x = (i << o)
      if (x != anarchy.flop(anarchy.flop(x))):
        messages.append("flop @ {}".format(x))
        result += 1
      if (x != anarchy.rev_scramble(anarchy.scramble(x))):
        messages.append("scramble @ {}".format(x))
        result += 1
      if (x != anarchy.prng(anarchy.rev_prng(x, seed), seed)):
        messages.append("prng @ {}".format(x))
        result += 1
      r = x
      p = x
      for j, y in enumerate(TEST_VALUES):
        if (
          x != anarchy.rev_swirl(
            anarchy.swirl(x, y),
            y
          )
        ):
          messages.append("swirl @ {}, {}".format(x, y))
          result += 1

        if (x != anarchy.fold(anarchy.fold(x, y), y)):
          messages.append("fold @ {}, {}".format(x, y))
          result += 1

        if (r != anarchy.scramble(anarchy.rev_scramble(r))):
          messages.append("chained scramble @ {}".format(r))
          result += 1

        r = anarchy.rev_scramble(r)
        if (p != anarchy.rev_prng(anarchy.prng(p, seed), seed)):
          messages.append("chained prng @ {}".format(p))
          result += 1

        p = anarchy.prng(p, seed)

    print("100% done")

  if result != 0:
    print("unit_ops failures:")
    for m in messages:
      print(m)

  return result

def tolerance(n_samples):
  """
  Computes a tolerance value based on a number of samples for testing
  pseudo-random functions.
  """
  return 1.2 / (10 ** (math.log10(n_samples) - 3))

def moments_test(
  samples,
  exp_mean,
  exp_stdev,
  label,
  messages,
  tol=None
):
  """
  Tests the given list of samples to make sure it has close to the given
  expected mean and standard deviation. Will append to the given messages
  list (with messages including the given label) upon failure, and
  returns the number of failures (0, 1, or 2). The exp_stdev can be given
  as None to skip that test. If no tolerance value is given, a tolerance
  will be computed automatically using the tolerance function.
  """
  result = 0
  if tol == None:
    tol = tolerance(len(samples))
  mean = sum(samples) / len(samples)

  if exp_mean == 0:
    pct = abs(mean) # not a percentage
  else:
    pct = abs(1 - mean / exp_mean) # a percentage

  if pct > tol:
    result += 1
    messages.append(
      f"Suspicious mean discrepancy from ({exp_mean:.4f}) for {label}:"
    + f" {mean:.4f} -> {100 * pct:.2f}%"
    )

  if exp_stdev != None:
    stdev = 0
    for s in samples:
      stdev += (s - mean)**2

    stdev /= len(samples) - 1
    stdev = stdev ** 0.5

    if exp_stdev == 0:
      pct = stdev # not a percentage
    else:
      pct = abs(1 - stdev / exp_stdev) # actually a percentage

    if pct > tol:
      result += 1
      messages.append(
        f"Suspicious stdev discrepancy from ({exp_stdev:.4f}) for"
      + f" {label}: {stdev:.4f} -> {100 * pct:.2f}%"
      )

  return result

def trapezoid_area(height, top, bottom):
  """
  Computes the area of a trapezoid with the given height and top/bottom
  lengths.
  """
  return (
      # triangle based on longer - shorter of the top/bottom
      0.5 * abs(top - bottom) * height
      # plus parallelogram based on shorter edge
    + min(top, bottom) * height
  )

def cdf_points(low, high):
  """
  Returns a list of N_CDF_BUCKETS + 1 evenly-distributed test points
  starting at low and ending at high.
  """
  return list(
    map(
      lambda x: low + (x/N_CDF_BUCKETS) * (high - low),
      range(N_CDF_BUCKETS))
  ) + [high]

def cdf_test(samples, cdf, test_points, label, messages, tol=None):
  """
  Tests that the cumulative distribution function of the given
  samples roughly matches the given expected cumulative distribution
  function (should be a function which accepts a number x and
  returns a probability of the result being smaller than x). This
  estimates the total area of the differences between the actual and
  expected CDFs sampling at each of the given test points, and then
  requires that the percentage that this represents of the area
  under the true CDF is below a certain threshold based on the
  number of samples. This function returns the number of failed
  tests (0 or 1) and if the test fails, it pushes a message (which
  includes the given label) onto the given messages array.

  If no tolerance values is given, an automatic tolerance will be
  computed based on the number of test_points.
  """
  result = 0;
  ns = len(samples)
  if tol == None:
    tol = tolerance(len(samples))

  ordered = sorted(samples)

  discrepancy_area = 0
  correct_area = 0
  obs_precount = 0
  for i, tp in enumerate(test_points):
      exp_precount = cdf(tp) * ns;
      while obs_precount < ns and ordered[obs_precount] < tp:
          obs_precount += 1

      # Compute overshoot at this test point
      overshoot = obs_precount - exp_precount;

      # Only for the 2nd+ test points...
      if i > 0:
        # compute top and bottom of (possibly twisted) trapezoid
        width = tp - test_points[i-1];

        # update correct area
        correct_area += trapezoid_area(width, prev_exp_pc, exp_precount)

        # update discrepancy area...
        if prev_overshoot > 0 == overshoot > 0:
            # it's a trapezoid; both sides either over- or under-shot.
            discrepancy_area += trapezoid_area(
                width,
                abs(prev_overshoot),
                abs(overshoot)
            )
        else:
            # it's two triangles; one side did the opposite of the other.
            if overshoot != 0:
                ratio = abs(prev_overshoot / overshoot)
                inflection = width * ratio / (1 + ratio)
            else:
                inflection = width
            discrepancy_area += (
                # triangle from prev test point to inflection point
                0.5 * abs(prev_overshoot) * inflection
                # triangle from inflection point to current test point
              + 0.5 * abs(overshoot) * (width - inflection)
            )

      # Update previous variables for our next step
      prev_exp_pc = exp_precount
      prev_obs_pc = obs_precount
      prev_overshoot = overshoot

  discrepancy = abs(discrepancy_area / correct_area);
  if discrepancy > tol:
      messages.append(
        f"Suspicious CDF area discrepancy from ({correct_area:.2f}) for"
      + f" {label}: {discrepancy_area:.2f} -> {100*discrepancy:.2f}%"
      );
      result += 1;

  return result;

PRG_COUNTER = 0
PRG_TOTAL = None
PRG_NOTIFY = None

def reset_progress(total):
  """
  Sets up to report progress out of the given total number of progress
  calls. Progress will be reported every 5% completion.
  """
  global PRG_COUNTER, PRG_TOTAL, PRG_NOTIFY
  PRG_COUNTER = 0
  PRG_TOTAL = total
  PRG_NOTIFY = total // 20

def progress():
  """
  Records 1 unit of progress towards the total specified when
  reset_progress was called, and if we just broke a new 5% completion
  milestone, a message is printed (with a carriage return at the end so
  that the next message will overwrite it).
  """
  global PRG_COUNTER, PRG_TOTAL, PRG_NOTIFY
  PRG_COUNTER += 1
  if PRG_COUNTER % PRG_NOTIFY == 0:
    pct = int(100 * PRG_COUNTER/PRG_TOTAL)
    print(f"{pct:.0f}% done", end='\r')

def finished():
  """
  Records the end of progress by printing '100% done' and moving the
  cursor onto the next line.
  """
  print("100% done")

@test
def report_tolerance():
  """
  Not a real test, just reports the tolerance that's going to be used for
  the distribution tests.
  """
  print(
    f"Using {N_SAMPLES} samples, the default tolerance will be"
  + f" {100*tolerance(N_SAMPLES):0.2f}%."
  )
  return 0

@test
def udist_distribution():
  """
  Tests the mean, standard deviation, and CDF for udist results using
  N_SAMPLES samples starting from each of a few different seeds.
  """
  result = 0
  messages = []

  # udist tests
  print("udist...")
  reset_progress(len(TEST_VALUES) * N_SAMPLES)
  for seed in TEST_VALUES:
    rng = seed
    samples = []
    for i in range(N_SAMPLES):
      progress()
      samples.append(anarchy.udist(rng))
      rng = anarchy.prng(rng, seed)

    result += moments_test(
      samples,
      0.5,
      1/12**0.5,
      f"udist(:{seed}:)",
      messages
    )

    result += cdf_test(
      samples,
      lambda x: x,
      cdf_points(0, 1),
      f"udist(:{seed}:)",
      messages
    )
  finished()

  print("udist sequential...")
  reset_progress(len(SEQ_SEEDS))
  samples = []
  for seed in SEQ_SEEDS:
    progress()
    samples.append(anarchy.udist(seed))

  result += moments_test(
    samples,
    0.5,
    1/12**0.5,
    f"udist(:sequential:)",
    messages
  )

  result += cdf_test(
    samples,
    lambda x: x,
    cdf_points(0, 1),
    f"udist(:sequential:)",
    messages
  )

  finished()

  # Final message
  if result != 0:
    print("udist failures:")
    for m in messages:
      print(m)

  return result

@test
def pgdist_distribution():
  """
  Like udist_distribution, but tests pgdist. Does not test the CDF.
  (TODO: That?)
  """
  result = 0
  messages = []

  # pgdist tests
  print("Testing pgdist distribution...")
  reset_progress(len(TEST_VALUES) * N_SAMPLES)
  for seed in TEST_VALUES:
    rng = seed
    samples = []
    for i in range(N_SAMPLES):
      progress()
      samples.append(anarchy.pgdist(rng))
      rng = anarchy.prng(rng, seed)

    result += moments_test(
      samples,
      0.5,
      1/6, # see js/anarchy_tests.js for derivation
      f"pgdist(:{seed}:)",
      messages
    )

    # TODO: CDF test here?
  finished()

  print("pgdist sequential...")
  reset_progress(len(SEQ_SEEDS))
  samples = []
  for seed in SEQ_SEEDS:
    progress()
    samples.append(anarchy.pgdist(seed))

  result += moments_test(
    samples,
    0.5,
    1/6,
    f"pgdist(:sequential:)",
    messages
  )

  finished()

  # Final message
  if result != 0:
    print("pgdist failures:")
    for m in messages:
      print(m)

  return result


@test
def flip_distribution():
  """
  Tests the mean and standard deviation for the flip function using a few
  different probabilities and a few different seeds at each probability.
  """
  result = 0
  messages = []

  # flip tests
  print("Testing flip distribution...")
  test_with = [0.005, 0.2, 0.5, 0.9, 0.99]
  reset_progress(
    (len(test_with) - 1) * len(TEST_VALUES) * N_SAMPLES
  + len(TEST_VALUES) * N_SAMPLES * 5 # the 0.005 gets more samples
  )
  for p in test_with:
    for seed in TEST_VALUES:
      rng = seed
      samples = []
      for i in range(N_SAMPLES if p != 0.005 else N_SAMPLES*5):
        progress()
        samples.append(anarchy.flip(p, rng))
        rng = anarchy.prng(rng, seed)

        # Simple approximation for correct standard deviation:
        exp_stdev = (
          (
            ((1000 * p) * math.pow(1 - p, 2))
            + ((1000 * (1-p)) * math.pow(p, 2))
          ) / 999
        ) ** 0.5

      result += moments_test(
        samples,
        p,
        exp_stdev,
        f"flip({p}, :{seed}:)",
        messages,
        None if p != 0.005 else tolerance(N_SAMPLES)
        # we allow 5x tolerance for the low-p flip...
      )

  finished()

  print("flip sequential...")
  reset_progress(len(SEQ_SEEDS))
  samples = []
  for seed in SEQ_SEEDS:
    progress()
    samples.append(anarchy.flip(0.5, seed))

  result += moments_test(
    samples,
    0.5,
    0.5,
    f"flip(0.5, :sequential:)",
    messages
  )

  finished()

  # Final message
  if result != 0:
    print("flip failures:")
    for m in messages:
      print(m)

  return result

@test
def idist_distribution():
  """
  Tests the mean, stdev, and CDF for idist using various low/high values,
  including inverted pairs.
  """
  result = 0
  messages = []

  print("Testing idist distribution...")
  lows = [0, -31, 1289, -7294712]
  highs = [0, -30, 1289482, -7298392]
  reset_progress(len(lows) * len(highs) * len(TEST_VALUES) * N_SAMPLES)
  for low in lows:
    for high in highs:
      for seed in TEST_VALUES:
        rng = seed
        samples = []
        for i in range(N_SAMPLES):
          progress()
          samples.append(anarchy.idist(rng, low, high))
          rng = anarchy.prng(rng, seed)

        if low == high:
          # No need for mean/stdev/cdf tests here
          if any(s != low for s in samples):
            result += 1
            messages.append(
              f"Non-fixed sample(s) for idist(:{seed}:, {low}, {high}):"
            + f" {[s for s in samples if s != low]}"
            )
        else:
          span = high - 1 - low
          exp_mean = low + span/2
          exp_stdev = (1/12**0.5)*abs(span)

          result += moments_test(
            samples,
            exp_mean,
            exp_stdev,
            f"idist(:{seed}:, {low}, {high})",
            messages
          )

          real_low = min(low, high)
          real_high = max(low, high)

          result += cdf_test(
            samples,
            lambda x: (
              min(1, ((x-real_low) / (real_high-real_low) ))
              if real_high > real_low + 1
              else (1 if x > real_low else 0)
            ), 
            [real_low, real_high+1]
              if high <= low+1
              else cdf_points(real_low, real_high),
            f"idist(:{seed}:, {low}, {high})",
            messages
          )

  finished()

  print("idist sequential...")
  reset_progress(len(SEQ_SEEDS))
  samples = []
  low = -12
  high = 10472
  span = high - 1 - low
  exp_mean = low + span/2
  exp_stdev = (1/12**0.5)*abs(span)
  for seed in SEQ_SEEDS:
    progress()
    samples.append(anarchy.idist(seed, low, high))

  result += moments_test(
    samples,
    exp_mean,
    exp_stdev,
    f"idist(:sequential:, {low}, {high})",
    messages
  )

  result += cdf_test(
    samples,
    lambda x: min(1, ((x-low) / (high-low) )), 
    cdf_points(low, high),
    f"idist(:{seed}:, {low}, {high})",
    messages
  )

  finished()

  # Final message
  if result != 0:
    print("idist failures:")
    for m in messages:
      print(m)

  return result

@test
def expdist_distribution():
  """
  Tests the distribution of expdist means, stdevs, and CDFs for several
  different seeds at each of several different shape values.
  """
  result = 0
  messages = []

  # expdist tests
  print("Testing expdist distribution...")
  shapes = [0.05, 0.5, 1, 1.5, 5]
  reset_progress(len(shapes) * len(TEST_VALUES) * N_SAMPLES)
  for shape in shapes:
    # Test with different shape (lambda) values
    for seed in TEST_VALUES:
      rng = seed
      samples = []

      exp_mean = 1/shape
      # expected stdev is the same as the expected mean for an
      # exponential distribution
      exp_stdev = exp_mean

      # compute samples
      for i in range(N_SAMPLES):
        progress()
        samples.append(anarchy.expdist(rng, shape))
        rng = anarchy.prng(rng, seed)

      result += moments_test(
        samples,
        exp_mean,
        exp_stdev,
        f"expdist(:{seed}:, {shape})",
        messages
      )

      result += cdf_test(
        samples,
        lambda x: 1 - math.exp(-shape * x),
        cdf_points(0, 2) + cdf_points(2.5, 30),
        f"expdist(:{seed}:, {shape})",
        messages
      )

  finished()

  print("expdist sequential...")
  reset_progress(len(SEQ_SEEDS))
  samples = []
  shape = 0.75
  exp_mean = 1/shape
  exp_stdev = exp_mean
  for seed in SEQ_SEEDS:
    progress()
    samples.append(anarchy.expdist(seed, shape))

  result += moments_test(
    samples,
    exp_mean,
    exp_stdev,
    f"expdist(:sequential:, {shape})",
    messages
  )

  result += cdf_test(
    samples,
    lambda x: 1 - math.exp(-shape * x),
    cdf_points(0, 2) + cdf_points(2.5, 30),
    f"expdist(:sequential:, {shape})",
    messages
  )

  finished()

  # Final message
  if result != 0:
    print("expdist failures:")
    for m in messages:
      print(m)

  return result

@test
def trexpdist_distribution():
  """
  Tests trexpdist results at several different shape values. Tests means
  and CDFS but not stdevs (TODO: that?).
  """
  result = 0
  messages = []

  shapes = [0.05, 0.5, 1, 1.5, 5]
  reset_progress(len(shapes) * len(TEST_VALUES) * N_SAMPLES)
  print("Testing trexpdist distribution...")
  for shape in shapes:
    # Test with different shape (lambda) values
    for seed in TEST_VALUES:
      rng = seed
      samples = []

      exp_mean = 1/shape - 1/(math.exp(shape) - 1)
      # TODO: What's the expected stdev here?
      exp_stdev = None

      # compute samples
      for i in range(N_SAMPLES):
        progress()
        samples.append(anarchy.trexpdist(rng, shape))
        rng = anarchy.prng(rng, seed)

      result += moments_test(
        samples,
        exp_mean,
        exp_stdev,
        f"trexpdist(:{seed}:, {shape})",
        messages
      )

      # Note: js/anarchy_tests.js contains a derivation of the truncated
      # CDF formula we're using here
      result += cdf_test(
        samples,
        lambda x: (1 - math.exp(-shape * x)) / (1 - math.exp(-shape)),
        cdf_points(0, 1),
        f"trexpdist(:{seed}:, {shape})",
        messages
      )

  finished()

  print("trexpdist sequential...")
  reset_progress(len(SEQ_SEEDS))
  samples = []
  shape = 0.75
  exp_mean = 1/shape - 1/(math.exp(shape) - 1)
  exp_stdev = None # TODO
  for seed in SEQ_SEEDS:
    progress()
    samples.append(anarchy.trexpdist(seed, shape))

  result += moments_test(
    samples,
    exp_mean,
    exp_stdev,
    f"trexpdist(:sequential:, {shape})",
    messages
  )

  result += cdf_test(
    samples,
    lambda x: (1 - math.exp(-shape * x)) / (1 - math.exp(-shape)),
    cdf_points(0, 1),
    f"trexpdist(:sequential:, {shape})",
    messages
  )

  finished()

  # Final message
  if result != 0:
    print("trexpdist failures:")
    for m in messages:
      print(m)

  return result

@test
def cohort_ops():
  result = 0
  messages = []
  cohort_sizes = [ 3, 12, 17, 32, 1024 ]
  for idx in range(len(cohort_sizes)):
    cs = cohort_sizes[idx]
    observed = {
      "interleave": {},
      "fold": {},
      "spin": {},
      "flop": {},
      "mix": {},
      "spread": {},
      "upend": {},
      "shuffle": {},
    }
    for i in range(cs):
      # interleave
      x = anarchy.cohort_interleave(i, cs)
      rc = anarchy.rev_cohort_interleave(x, cs)
      if (i != rc):
        messages.append("interleave({}, {})→{}→{}".format( i, cs, x, rc))
        result += 1

      v = str(x)
      if v in observed["interleave"]:
        observed["interleave"][v] += 1
      else:
        observed["interleave"][v] = 1

    for j, seed in enumerate(TEST_VALUES):
      observed["fold"][str(j)] = {}
      observed["spin"][str(j)] = {}
      observed["flop"][str(j)] = {}
      observed["mix"][str(j)] = {}
      observed["spread"][str(j)] = {}
      observed["upend"][str(j)] = {}
      observed["shuffle"][str(j)] = {}
      for i in range(cs):
        # fold
        x = anarchy.cohort_fold(i, cs, seed)
        rc = anarchy.rev_cohort_fold(x, cs, seed)
        if i != rc:
          messages.append(
            "fold({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["fold"][str(j)]:
          observed["fold"][str(j)][v] += 1
        else:
          observed["fold"][str(j)][v] = 1

        # spin
        x = anarchy.cohort_spin(i, cs, seed)
        rc = anarchy.rev_cohort_spin(x, cs, seed)
        if i != rc:
          messages.append(
            "spin({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["spin"][str(j)]:
          observed["spin"][str(j)][v] += 1
        else:
          observed["spin"][str(j)][v] = 1

        # flop
        x = anarchy.cohort_flop(i, cs, seed)
        rc = anarchy.cohort_flop(x, cs, seed)
        if i != rc:
          messages.append("flop({}, {}, {})→{}→{}".format(i, cs, seed, x, rc))
          result += 1

        v = str(x)
        if v in observed["flop"][str(j)]:
          observed["flop"][str(j)][v] += 1
        else:
          observed["flop"][str(j)][v] = 1

        # mix
        x = anarchy.cohort_mix(i, cs, seed)
        rc = anarchy.rev_cohort_mix(x, cs, seed)
        if i != rc:
          messages.append(
            "mix({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["mix"][str(j)]:
          observed["mix"][str(j)][v] += 1
        else:
          observed["mix"][str(j)][v] = 1

        # spread
        x = anarchy.cohort_spread(i, cs, seed)
        rc = anarchy.rev_cohort_spread(x, cs, seed)
        if i != rc:
          messages.append(
            "spread({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["spread"][str(j)]:
          observed["spread"][str(j)][v] += 1
        else:
          observed["spread"][str(j)][v] = 1

        # upend
        x = anarchy.cohort_upend(i, cs, seed)
        rc = anarchy.cohort_upend(x, cs, seed)
        if i != rc:
          messages.append(
            "upend({}, {}, {})→{}→{}".format(i, cs, i, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["upend"][str(j)]:
          observed["upend"][str(j)][v] += 1
        else:
          observed["upend"][str(j)][v] = 1

        # shuffle
        x = anarchy.cohort_shuffle(i, cs, seed)
        rc = anarchy.rev_cohort_shuffle(x, cs, seed)
        if i != rc:
          messages.append(
            "shuffle({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["shuffle"][str(j)]:
          observed["shuffle"][str(j)][v] += 1
        else:
          observed["shuffle"][str(j)][v] = 1


    for prp in observed:
      if prp == "interleave":
        for i in range(cs):
          v = str(i)
          if v in observed[prp]:
            count = observed[prp][v]
          else:
            count = 0

          if count != 1:
            messages.append(
              "{}({}, {}) found {}".format(prp, i, cs, count)
            )
            result += 1

      else:
        for j, seed in enumerate(TEST_VALUES):
          k = str(j)
          for i in range(cs):
            v = str(i)
            count = observed[prp][k].get(v, 0)
            if count != 1:
              messages.append(
                "{}({}, {}, {}) found {}".format(prp, i, cs, seed, count)
              )
              result += 1


  if result != 0:
    print("cohort_ops failures:")
    for m in messages:
      print('  ' + m)

  return result

def run_value_tests():
  print("Starting value tests...")
  for t in VALUE_TESTS:
    test_count = len(VALUE_TESTS[t])
    passed = 0
    for index, sub_t in enumerate(VALUE_TESTS[t]):
      if sub_t[0] == sub_t[1]:
        passed += 1
      else:
        print("Test failed: {}.{}".format(t, index))
        print("  expected: {} got: {}".format(sub_t[1], sub_t[0]))

    print("Suite '{}': passed {} / {}".format(t, passed, test_count))

  print("Done with value tests.")

def run_exec_tests():
  for t in EXEC_TESTS:
    result = EXEC_TESTS[t]()
    if result != 0:
      print("Test '{}' failed {} sub-tests.".format(t, result))
    else:
      print("Test '{}' succeeded.".format(t))

# do it!
if __name__ == "__main__":
  run_value_tests()
  run_exec_tests()
