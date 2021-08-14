"""
Testing module for anarchy reversible RNG package.

test.py
"""

import math
import sys

from . import rng, cohort


# Straightforward value tests...

def test_posmod():
    assert rng.posmod(-1, 7) == 6
    assert rng.posmod(0, 11) == 0
    assert rng.posmod(3, 11) == 3
    assert rng.posmod(-13, 11) == 9
    assert rng.posmod(15, 11) == 4
    assert rng.posmod(115, 10) == 5
    assert rng.posmod(115, 100) == 15


def test_mask():
    assert rng.mask(0) == 0
    assert rng.mask(1) == 1
    assert rng.mask(2) == 3
    assert rng.mask(4) == 15
    assert rng.mask(10) == 1023


def test_byte_mask():
    assert rng.byte_mask(0) == 255
    assert rng.byte_mask(1) == 65280
    assert rng.byte_mask(2) == 16711680


def test_swirl():
    assert rng.swirl(2, 1) == 1
    assert rng.swirl(4, 1) == 2
    assert rng.swirl(8, 2) == 2
    assert rng.swirl(1, 1) == 0x8000000000000000
    assert rng.swirl(2, 2) == 0x8000000000000000
    assert rng.swirl(1, 2) == 0x4000000000000000
    assert rng.rev_swirl(1, 2) == 4
    assert rng.rev_swirl(1, 3) == 8
    assert rng.rev_swirl(2, 2) == 8
    assert rng.rev_swirl(0x8000000000000000, 1) == 1
    assert rng.rev_swirl(0x8000000000000000, 2) == 2
    assert rng.rev_swirl(0x8000000000000000, 3) == 4
    assert rng.rev_swirl(0x4000000000000000, 3) == 2
    assert rng.rev_swirl(0x0000000000101030, 1) == 0x0000000000202060
    assert rng.rev_swirl(rng.swirl(1098301, 17), 17) == 1098301


def test_fold():
    # TODO: Verify these!
    assert rng.fold(22908, 7) == 50375224738208124
    assert rng.fold(18201, 18) == 1280781512777680665
    assert rng.fold(rng.fold(18201, 18), 18) == 18201
    assert rng.fold(rng.fold(89348393, 1289), 1289) == 89348393


def test_flop():
    # TODO: Verify these!
    assert rng.flop(0xf0f0f0f0) == 0x0f0f0f0f
    assert rng.flop(0x0f0f0f0f) == 0xf0f0f0f0
    assert rng.flop(22908) == 38343
    assert rng.flop(18201) == 29841
    assert rng.flop(rng.flop(3892389)) == 3892389
    assert rng.flop(rng.flop(489248448)) == 489248448


def test_scramble():
    assert (
        rng.scramble(rng.rev_swirl(0x03040610 | 0x40004001, 1))
     == 0x40004001
    )
    assert (
        rng.rev_scramble(0x40004001)
     == rng.rev_swirl(0x03040610 | 0x40004001, 1)
    )
    assert rng.rev_scramble(rng.scramble(17)) == 17
    assert rng.rev_scramble(rng.scramble(8493489)) == 8493489
    assert rng.scramble(rng.rev_scramble(8493489)) == 8493489


def test_prng():
    assert rng.prng(489348, 373891) == 18107188676709054266
    assert rng.rev_prng(18107188676709054266, 373891) == 489348
    assert rng.rev_prng(1766311112, 373891) == 14566213110237565854
    assert rng.prng(0, 0) == 15132939213242511212
    assert rng.rev_prng(15132939213242511212, 0) == 0
    assert rng.prng(rng.rev_prng(1782, 39823), 39823) == 1782
    assert rng.rev_prng(rng.prng(1782, 39823), 39823) == 1782


def test_lfsr():
    # TODO: Verify these!
    assert rng.lfsr(489348) == 244674
    assert rng.lfsr(1766932808) == 883466404


def test_uniform():
    assert rng.uniform(0) == 0.842373086655968
    assert rng.uniform(8329801) == 0.5186867891411134
    assert rng.uniform(58923) == 0.9716616308000062


def test_normalish():
    assert rng.normalish(0) == 0.6184944203669203
    assert rng.normalish(8329801) == 0.4333781836142186
    assert rng.normalish(58923) == 0.6939341836590818


def test_integer():
    assert rng.integer(0, 0, 1) == 0
    assert rng.integer(0, 3, 25) == 21
    assert rng.integer(58923, 3, 25) == 24
    assert rng.integer(58923, -2, -4) == -4


def test_exponential():
    assert rng.exponential(0, 0.5) == 3.6950486923768895
    assert rng.exponential(8329801, 0.5) == 1.4624741095323832
    assert rng.exponential(58923, 1.5) == 2.375692393655211


def test_truncated_exponential():
    assert rng.truncated_exponential(0, 0.5) == 0.6950486923768895
    assert rng.truncated_exponential(8329801, 1.5) == 0.4874913698441277
    assert rng.truncated_exponential(58923, 2.5) == 0.4254154361931266


def test_cohorts():
    assert cohort.cohort(17, 3) == 5
    assert cohort.cohort(10, 10) == 1
    assert cohort.cohort(9, 10) == 0
    assert cohort.cohort_inner(17, 3) == 2
    assert cohort.cohort_inner(10, 10) == 0
    assert cohort.cohort_inner(9, 10) == 9
    assert cohort.cohort_and_inner(9, 10)[0] == 0
    assert cohort.cohort_and_inner(9, 10)[1] == 9
    assert cohort.cohort_outer(0, 3, 112) == 3
    assert cohort.cohort_outer(1, 3, 112) == 115
    assert cohort.cohort_outer(-1, 3, 112) == 18446744073709551507


def test_cohort_interleave():
    assert cohort.cohort_interleave(3, 12) == 6
    assert cohort.cohort_interleave(7, 12) == 9


def test_cohort_spin():
    assert cohort.cohort_spin(0, 2, 1048239) == 1
    assert cohort.rev_cohort_spin(1, 2, 1048239) == 0


def test_cohort_mix():
    assert cohort.cohort_mix(0, 3, 0) == 2
    assert cohort.cohort_mix(1, 3, 0) == 1
    assert cohort.cohort_mix(2, 3, 0) == 0
    assert cohort.rev_cohort_mix(2, 3, 0) == 0
    assert cohort.rev_cohort_mix(1, 3, 0) == 1
    assert cohort.rev_cohort_mix(0, 3, 0) == 2


def test_cohort_fold():
    assert cohort.cohort_fold(0, 3, 0) == 0
    assert cohort.cohort_fold(1, 3, 0) == 2
    assert cohort.cohort_fold(2, 3, 0) == 1
    assert cohort.rev_cohort_fold(0, 3, 0) == 0
    assert cohort.rev_cohort_fold(1, 3, 0) == 2
    assert cohort.rev_cohort_fold(2, 3, 0) == 1


def test_cohort_shuffle():
    assert cohort.cohort_shuffle(0, 3, 17) == 0
    assert cohort.cohort_shuffle(1, 3, 17) == 2
    assert cohort.cohort_shuffle(2, 3, 17) == 1
    assert cohort.rev_cohort_shuffle(0, 3, 17) == 0
    assert cohort.rev_cohort_shuffle(2, 3, 17) == 1
    assert cohort.rev_cohort_shuffle(1, 3, 17) == 2


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

PROGRESS = 0
PROGRESS_LABEL = "Progress"
TOTAL_WORK = None


def reset_progress():
    """
    Resets the progress counter.
    """
    global PROGRESS
    PROGRESS = 0


def start_progress(label, work_units):
    """
    Sets up for progress printing by declaring the label and the total
    units of work to be accomplished. Sets PROGRESS to 0 as well.
    """
    global PROGRESS, PROGRESS_LABEL, TOTAL_WORK
    PROGRESS = 0
    PROGRESS_LABEL = label
    TOTAL_WORK = work_units


def progress():
    """
    Increments the progress counter and prints a message if that
    increment achieved 1% of progress.
    """
    global PROGRESS
    if PROGRESS == 0:
        print(f"\n{PROGRESS_LABEL}: 0%", end="\r")
        sys.stdout.flush()
    elif PROGRESS == TOTAL_WORK - 1:
        print(f"{PROGRESS_LABEL}: 100%") # now we introduce a newline
        sys.stdout.flush()
    elif TOTAL_WORK is not None:
        old_pct = (PROGRESS * 100) // TOTAL_WORK
        new_pct = ((PROGRESS + 1) * 100) // TOTAL_WORK
        if old_pct != new_pct:
            print(f"{PROGRESS_LABEL}: {new_pct}%", end='\r')
    PROGRESS += 1


def loudtest(test):
    """
    Sets up a test as a loud test that can print stuff during testing.
    """
    cargidx = -1

    def decorated(*args):
        capsys = args[cargidx]
        before = args[:cargidx]
        if cargidx == -1:
            args = before
        else:
            args = before + args[cargidx + 1:]

        with capsys.disabled():
            result = test(*args)

        return result

    # Grab info on original function signature
    fname = test.__name__
    nargs = test.__code__.co_argcount
    argnames = list(test.__code__.co_varnames[:nargs])
    if "capsys" not in argnames:
        argnames.append("capsys")
    else:
        cargidx = argnames.index("capsys")

    # Create a function with the same signature, so that pytest won't
    # break
    code = """\
def {name}({args}):
    return decorated({args})
""".format(name=fname, args=', '.join(argnames))
    env = {"decorated": decorated}
    exec(code, env, env)
    result = env[fname]

    # Preserve docstring
    result.__doc__ = test.__doc__

    return result


@loudtest
def test_unit_ops():
    seed = 129873918231
    start_progress("ops", 5 * (1 << 12) * len(TEST_VALUES))
    for o in (0, 12, 24, 36, 48):
        for i in range(1 << 12):
            x = (i << o)
            assert x == rng.flop(rng.flop(x))
            assert x == rng.rev_scramble(rng.scramble(x))
            assert x == rng.prng(rng.rev_prng(x, seed), seed)
            r = x
            p = x
            for j, y in enumerate(TEST_VALUES):
                assert x == rng.rev_swirl(rng.swirl(x, y), y)
                assert x == rng.fold(rng.fold(x, y), y)
                assert r == rng.scramble(rng.rev_scramble(r))
                r = rng.rev_scramble(r)
                assert p == rng.rev_prng(rng.prng(p, seed), seed)
                p = rng.prng(p, seed)
                progress()


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
    tol=None
):
    """
    Tests the given list of samples to make sure it has close to the given
    expected mean and standard deviation. Uses assertions to indicate
    success/failure. The exp_stdev can be given as None to skip that
    test. If no tolerance value is given, a tolerance will be computed
    automatically using the tolerance function. The label is used in
    failure messages.
    """
    if tol is None:
        tol = tolerance(len(samples))
    mean = sum(samples) / len(samples)

    if exp_mean == 0:
        pct = abs(mean) # not a percentage
    else:
        pct = abs(1 - mean / exp_mean) # a percentage

    assert pct <= tol, (
        f"Suspicious mean discrepancy from ({exp_mean:.4f}) for {label}:"
        f" {mean:.4f} -> {100 * pct:.2f}%"
    )

    if exp_stdev is not None:
        stdev = 0
        for s in samples:
            stdev += (s - mean)**2

        stdev /= len(samples) - 1
        stdev = stdev ** 0.5

        if exp_stdev == 0:
            pct = stdev # not a percentage
        else:
            pct = abs(1 - stdev / exp_stdev) # actually a percentage

        assert pct <= tol, (
            f"Suspicious stdev discrepancy from ({exp_stdev:.4f}) for"
            f" {label}: {stdev:.4f} -> {100 * pct:.2f}%"
        )


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
            lambda x: low + (x / N_CDF_BUCKETS) * (high - low),
            range(N_CDF_BUCKETS)
        )
    ) + [high]


def cdf_test(samples, cdf, test_points, label, tol=None):
    """
    Tests that the cumulative distribution function of the given
    samples roughly matches the given expected cumulative distribution
    function (should be a function which accepts a number x and
    returns a probability of the result being smaller than x). This
    estimates the total area of the differences between the actual and
    expected CDFs sampling at each of the given test points, and then
    requires that the percentage that this represents of the area
    under the true CDF is below a certain threshold based on the
    number of samples. This function uses asserts to test the outcome;
    the label is used for messaging.

    If no tolerance values is given, an automatic tolerance will be
    computed based on the number of test_points.
    """
    ns = len(samples)
    if tol is None:
        tol = tolerance(len(samples))

    ordered = sorted(samples)

    discrepancy_area = 0
    correct_area = 0
    obs_precount = 0

    # Initialize previous variables for our next step
    prev_exp_pc = 0
    prev_overshoot = 0

    for i, tp in enumerate(test_points):
        exp_precount = cdf(tp) * ns
        while obs_precount < ns and ordered[obs_precount] < tp:
            obs_precount += 1

        # Compute overshoot at this test point
        overshoot = obs_precount - exp_precount

        # Only for the 2nd+ test points...
        if i > 0:
            # compute top and bottom of (possibly twisted) trapezoid
            width = tp - test_points[i - 1]

            # update correct area
            correct_area += trapezoid_area(width, prev_exp_pc, exp_precount)

            # update discrepancy area...
            if prev_overshoot > 0 == overshoot > 0:
                # it's a trapezoid; both sides either over- or
                # under-shot.
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
                    # triangle from prev test point to
                    # inflection point
                    0.5 * abs(prev_overshoot) * inflection
                    # triangle from inflection point to current
                    # test point
                  + 0.5 * abs(overshoot) * (width - inflection)
                )

        # Update previous variables for our next step
        prev_exp_pc = exp_precount
        prev_overshoot = overshoot

    discrepancy = abs(discrepancy_area / correct_area)
    assert discrepancy <= tol, (
        f"Suspicious CDF area discrepancy from ({correct_area:.2f}) for"
        f" {label}: {discrepancy_area:.2f} -> {100*discrepancy:.2f}%"
    )


@loudtest
def test_report_tolerance():
    """
    Not a real test, just reports the tolerance that's going to be used for
    the distribution tests.
    """
    print(
        f"\nUsing {N_SAMPLES} samples, the default tolerance will be"
        f" {100*tolerance(N_SAMPLES):0.2f}%."
    )


@loudtest
def test_uniform_distribution():
    """
    Tests the mean, standard deviation, and CDF for uniform results using
    N_SAMPLES samples starting from each of a few different seeds.
    """
    start_progress("uniform::managed", len(TEST_VALUES) * N_SAMPLES)
    for seed in TEST_VALUES:
        rand = seed
        samples = []
        for i in range(N_SAMPLES):
            samples.append(rng.uniform(rand))
            rand = rng.prng(rand, seed)
            progress()

        moments_test(
            samples,
            0.5,
            1 / 12 ** 0.5,
            f"uniform(:{seed}:)"
        )

        cdf_test(
            samples,
            lambda x: x,
            cdf_points(0, 1),
            f"uniform(:{seed}:)"
        )

    samples = []
    start_progress("uniform::sequential", len(SEQ_SEEDS))
    for seed in SEQ_SEEDS:
        samples.append(rng.uniform(seed))
        progress()

    moments_test(
        samples,
        0.5,
        1 / 12 ** 0.5,
        "uniform(:sequential:)"
    )

    cdf_test(
        samples,
        lambda x: x,
        cdf_points(0, 1),
        "uniform(:sequential:)"
    )


@loudtest
def test_normalish_distribution():
    """
    Like `test_uniform_distribution`, but tests normalish. Does not test
    the CDF. (TODO: That?)
    """
    start_progress("normalish::managed", len(TEST_VALUES) * N_SAMPLES)
    for seed in TEST_VALUES:
        rand = seed
        samples = []
        for i in range(N_SAMPLES):
            samples.append(rng.normalish(rand))
            rand = rng.prng(rand, seed)
            progress()

        moments_test(
            samples,
            0.5,
            1 / 6, # see js/anarchy_tests.js for derivation
            f"normalish(:{seed}:)"
        )

    # TODO: CDF test here?

    samples = []
    start_progress("normalish::sequential", len(SEQ_SEEDS))
    for seed in SEQ_SEEDS:
        samples.append(rng.normalish(seed))
        progress()

    moments_test(
        samples,
        0.5,
        1 / 6,
        "normalish(:sequential:)"
    )


@loudtest
def test_flip_distribution():
    """
    Tests the mean and standard deviation for the flip function using a few
    different probabilities and a few different seeds at each probability.
    """
    test_with = [0.005, 0.2, 0.5, 0.9, 0.99]
    start_progress(
        "flip::managed",
        (len(test_with) - 1) * len(TEST_VALUES) * N_SAMPLES
      + len(TEST_VALUES) * N_SAMPLES * 5
    )
    for p in test_with:
        for seed in TEST_VALUES:
            rand = seed
            samples = []
            for i in range(N_SAMPLES if p != 0.005 else N_SAMPLES * 5):
                samples.append(rng.flip(p, rand))
                rand = rng.prng(rand, seed)
                progress()

            # Simple approximation for correct standard deviation:
            exp_stdev = (
                (
                    ((1000 * p) * math.pow(1 - p, 2))
                    + ((1000 * (1 - p)) * math.pow(p, 2))
                ) / 999
            ) ** 0.5

            moments_test(
                samples,
                p,
                exp_stdev,
                f"flip({p}, :{seed}:)",
                tol=(None if p != 0.005 else tolerance(N_SAMPLES))
                # we allow 5x tolerance for the low-p flip...
            )

    samples = []
    start_progress("flip::sequential", len(SEQ_SEEDS))
    for seed in SEQ_SEEDS:
        samples.append(rng.flip(0.5, seed))
        progress()

    moments_test(
        samples,
        0.5,
        0.5,
        "flip(0.5, :sequential:)"
    )


@loudtest
def test_integer_distribution():
    """
    Tests the mean, stdev, and CDF for `anarchy.rng.integer` using
    various low/high values, including inverted pairs.
    """
    lows = [0, -31, 1289, -7294712]
    highs = [0, -30, 1289482, -7298392]
    start_progress(
        "integer::managed",
        len(lows) * len(highs) * len(TEST_VALUES) * N_SAMPLES
    )
    for low in lows:
        for high in highs:
            for seed in TEST_VALUES:
                rand = seed
                samples = []
                for i in range(N_SAMPLES):
                    samples.append(rng.integer(rand, low, high))
                    rand = rng.prng(rand, seed)
                    progress()

                if low == high:
                    # No need for mean/stdev/cdf tests here
                    assert all(s == low for s in samples), (
                        f"Non-fixed sample(s) for integer(:{seed}:,"
                        f" {low}, {high}):"
                        f" {[s for s in samples if s != low]}"
                    )
                else:
                    span = high - 1 - low
                    exp_mean = low + span / 2
                    exp_stdev = (1 / (12 ** 0.5)) * abs(span)

                    moments_test(
                        samples,
                        exp_mean,
                        exp_stdev,
                        f"integer(:{seed}:, {low}, {high})"
                    )

                    real_low = min(low, high)
                    real_high = max(low, high)

                    cdf_test(
                        samples,
                        lambda x: (
                            min(1, ((x - real_low) / (real_high - real_low) ))
                            if real_high > real_low + 1
                            else (1 if x > real_low else 0)
                        ),
                        [real_low, real_high + 1]
                            if high <= low + 1
                            else cdf_points(real_low, real_high),
                        f"integer(:{seed}:, {low}, {high})"
                    )

    samples = []
    low = -12
    high = 10472
    span = high - 1 - low
    exp_mean = low + span / 2
    exp_stdev = (1 / (12 ** 0.5)) * abs(span)
    start_progress("integer::sequential", len(SEQ_SEEDS))
    for seed in SEQ_SEEDS:
        samples.append(rng.integer(seed, low, high))
        progress()

    moments_test(
        samples,
        exp_mean,
        exp_stdev,
        f"integer(:sequential:, {low}, {high})"
    )

    cdf_test(
        samples,
        lambda x: min(1, ((x - low) / (high - low) )),
        cdf_points(low, high),
        f"integer(:{seed}:, {low}, {high})"
    )


@loudtest
def test_exponential_distribution():
    """
    Tests the distribution of exponential means, stdevs, and CDFs for
    several different seeds at each of several different shape values.
    """
    shapes = [0.05, 0.5, 1, 1.5, 5]
    start_progress(
        "exponential::managed",
        len(shapes) * len(TEST_VALUES) * N_SAMPLES
    )
    for shape in shapes:
        # Test with different shape (lambda) values
        for seed in TEST_VALUES:
            rand = seed
            samples = []

            exp_mean = 1 / shape
            # expected stdev is the same as the expected mean for an
            # exponential distribution
            exp_stdev = exp_mean

            # compute samples
            for i in range(N_SAMPLES):
                samples.append(rng.exponential(rand, shape))
                rand = rng.prng(rand, seed)
                progress()

            moments_test(
                samples,
                exp_mean,
                exp_stdev,
                f"exponential(:{seed}:, {shape})"
            )

            cdf_test(
                samples,
                lambda x: 1 - math.exp(-shape * x),
                cdf_points(0, 2) + cdf_points(2.5, 30),
                f"exponential(:{seed}:, {shape})"
            )

    samples = []
    shape = 0.75
    exp_mean = 1 / shape
    exp_stdev = exp_mean
    start_progress("exponential::sequential", len(SEQ_SEEDS))
    for seed in SEQ_SEEDS:
        samples.append(rng.exponential(seed, shape))
        progress()

    moments_test(
        samples,
        exp_mean,
        exp_stdev,
        f"exponential(:sequential:, {shape})"
    )

    cdf_test(
        samples,
        lambda x: 1 - math.exp(-shape * x),
        cdf_points(0, 2) + cdf_points(2.5, 30),
        f"exponential(:sequential:, {shape})"
    )


@loudtest
def test_truncated_exponential_distribution():
    """
    Tests `anarchy.rng.truncated_exponential` results at several
    different shape values. Tests means and CDFS but not stdevs (TODO:
    that?).
    """
    shapes = [0.05, 0.5, 1, 1.5, 5]
    start_progress(
        "truncated_exponential::managed",
        len(shapes) * len(TEST_VALUES) * N_SAMPLES
    )
    for shape in shapes:
        # Test with different shape (lambda) values
        for seed in TEST_VALUES:
            rand = seed
            samples = []

            exp_mean = (1 / shape) - (1 / (math.exp(shape) - 1))
            # TODO: What's the expected stdev here?
            exp_stdev = None

            # compute samples
            for i in range(N_SAMPLES):
                samples.append(rng.truncated_exponential(rand, shape))
                rand = rng.prng(rand, seed)
                progress()

            moments_test(
                samples,
                exp_mean,
                exp_stdev,
                f"truncated_exponential(:{seed}:, {shape})"
            )

            # Note: js/anarchy_tests.js contains a derivation of the truncated
            # CDF formula we're using here
            cdf_test(
                samples,
                lambda x: (1 - math.exp(-shape * x)) / (1 - math.exp(-shape)),
                cdf_points(0, 1),
                f"truncated_exponential(:{seed}:, {shape})"
            )

    samples = []
    shape = 0.75
    exp_mean = (1 / shape) - (1 / (math.exp(shape) - 1))
    exp_stdev = None # TODO
    start_progress("truncated_exponential::sequential", len(SEQ_SEEDS))
    for seed in SEQ_SEEDS:
        samples.append(rng.truncated_exponential(seed, shape))

    moments_test(
        samples,
        exp_mean,
        exp_stdev,
        f"truncated_exponential(:sequential:, {shape})"
    )

    cdf_test(
        samples,
        lambda x: (1 - math.exp(-shape * x)) / (1 - math.exp(-shape)),
        cdf_points(0, 1),
        f"truncated_exponential(:sequential:, {shape})"
    )


@loudtest
def test_cohort_ops():
    """
    Tests for the cohort operations.
    """
    cohort_sizes = [ 3, 12, 17, 32, 1024 ]
    start_progress(
        "cohort_ops",
        2 * sum(cohort_sizes) * (len(TEST_VALUES) + 1)
    )
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
            x = cohort.cohort_interleave(i, cs)
            rc = cohort.rev_cohort_interleave(x, cs)
            assert i == rc, f"interleave({i}, {cs})→{x}→{rc}"

            v = str(x)
            if v in observed["interleave"]:
                observed["interleave"][v] += 1
            else:
                observed["interleave"][v] = 1
            progress()

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
                x = cohort.cohort_fold(i, cs, seed)
                rc = cohort.rev_cohort_fold(x, cs, seed)
                assert i == rc, f"fold({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["fold"][str(j)]:
                    observed["fold"][str(j)][v] += 1
                else:
                    observed["fold"][str(j)][v] = 1

                # spin
                x = cohort.cohort_spin(i, cs, seed)
                rc = cohort.rev_cohort_spin(x, cs, seed)
                assert i == rc, f"spin({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["spin"][str(j)]:
                    observed["spin"][str(j)][v] += 1
                else:
                    observed["spin"][str(j)][v] = 1

                # flop
                x = cohort.cohort_flop(i, cs, seed)
                rc = cohort.cohort_flop(x, cs, seed)
                assert i == rc, f"flop({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["flop"][str(j)]:
                    observed["flop"][str(j)][v] += 1
                else:
                    observed["flop"][str(j)][v] = 1

                # mix
                x = cohort.cohort_mix(i, cs, seed)
                rc = cohort.rev_cohort_mix(x, cs, seed)
                assert i == rc, f"mix({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["mix"][str(j)]:
                    observed["mix"][str(j)][v] += 1
                else:
                    observed["mix"][str(j)][v] = 1

                # spread
                x = cohort.cohort_spread(i, cs, seed)
                rc = cohort.rev_cohort_spread(x, cs, seed)
                assert i == rc, f"spread({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["spread"][str(j)]:
                    observed["spread"][str(j)][v] += 1
                else:
                    observed["spread"][str(j)][v] = 1

                # upend
                x = cohort.cohort_upend(i, cs, seed)
                rc = cohort.cohort_upend(x, cs, seed)
                assert i == rc, f"upend({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["upend"][str(j)]:
                    observed["upend"][str(j)][v] += 1
                else:
                    observed["upend"][str(j)][v] = 1

                # shuffle
                x = cohort.cohort_shuffle(i, cs, seed)
                rc = cohort.rev_cohort_shuffle(x, cs, seed)
                assert i == rc, f"shuffle({i}, {cs}, {seed})→{x}→{rc}"

                v = str(x)
                if v in observed["shuffle"][str(j)]:
                    observed["shuffle"][str(j)][v] += 1
                else:
                    observed["shuffle"][str(j)][v] = 1
                progress()

        for prp in observed:
            if prp == "interleave":
                for i in range(cs):
                    v = str(i)
                    if v in observed[prp]:
                        count = observed[prp][v]
                    else:
                        count = 0

                    assert count == 1, f"{prp}({i}, {cs}) found {count}"
                    progress()

            else:
                for j, seed in enumerate(TEST_VALUES):
                    k = str(j)
                    for i in range(cs):
                        v = str(i)
                        count = observed[prp][k].get(v, 0)
                        assert count == 1, (
                            f"{prp}({i}, {cs}, {seed}) found {count}"
                        )
                        progress()


if __name__ == "__main__":
    import pytest
    sys.exit(pytest.main(["--pyargs", "anarchy.test"]))
