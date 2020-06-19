// anarchy_tests.js
// Tests for the anarchy reversible chaos library.
/* jshint esversion: 6 */

import * as anarchy from "./anarchy.mjs";

function display_message(m) {
    document.body.innerHTML += "<div>" + m + "</div>";
}

let VALUE_TESTS = {
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
        [ anarchy.swirl(1, 1), 0x80000000 ],
        [ anarchy.swirl(2, 2), 0x80000000 ],
        [ anarchy.swirl(1, 2), 0x40000000 ],
        [ anarchy.rev_swirl(1, 2), 4 ],
        [ anarchy.rev_swirl(1, 3), 8 ],
        [ anarchy.rev_swirl(2, 2), 8 ],
        [ anarchy.rev_swirl(0x80000000, 1), 1 ],
        [ anarchy.rev_swirl(0x80000000, 2), 2 ],
        [ anarchy.rev_swirl(0x80000000, 3), 4 ],
        [ anarchy.rev_swirl(0x40000000, 3), 2 ],
        [ anarchy.rev_swirl(0x00101030, 1), 0x00202060 ],
        [
            anarchy.rev_swirl(anarchy.swirl(1098301, 17), 17),
            1098301
        ],
    ],
    "fold": [
        // TODO: Verify these!
        [ anarchy.fold(22908, 7), 3002620284 ],
        [ anarchy.fold(18201, 18), 3326101273 ],
        [ anarchy.fold(anarchy.fold(18201, 18), 18), 18201 ],
        [ anarchy.fold(anarchy.fold(89348393, 1289), 1289), 89348393 ],
    ],
    "flop": [
        // TODO: Verify these!
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
        // TODO: Verify these!
        [ anarchy.prng(489348, 373891), 1766311112 ],
        [ anarchy.rev_prng(1766311112, 373891), 489348 ],
        [ anarchy.prng(0, 0), 2697180397 ],
        [ anarchy.rev_prng(2697180397, 0), 0 ],
        [ anarchy.prng(anarchy.rev_prng(1782, 39823), 39823), 1782 ],
        [ anarchy.rev_prng(anarchy.prng(1782, 39823), 39823), 1782 ],
    ],
    "lfsr": [
        // TODO: Verify these!
        [ anarchy.lfsr(489348, 373891), 244674 ],
        [ anarchy.lfsr(1766932808, 373891), 883466404 ],
    ],
    "udist": [
        // TODO: Verify and/or fix these!
        [ anarchy.udist(0), 0.4741466253923192 ],
        [ anarchy.udist(8329801), 0.911844237693452 ],
        [ anarchy.udist(58923), 0.03562796004493369 ],
    ],
    "idist": [
        // TODO: Verify and/or fix these!
        [ anarchy.idist(0, 0, 1), 0 ],
        [ anarchy.idist(0, 3, 25), 13 ],
        [ anarchy.idist(58923, 3, 25), 3 ],
        [ anarchy.idist(58923, -2, -4), -3 ],
    ],
    "expdist": [
        // TODO: Verify and/or fix these!
        [ anarchy.expdist(0, 0.5), 1.4924773377018894 ],
        [ anarchy.expdist(8329801, 0.5), 0.18457219099442898 ],
        [ anarchy.expdist(58923, 1.5), 2.2230830365762837 ],
    ],
    "trexpdist": [
        // TODO: Verify and/or fix these!
        [ anarchy.trexpdist(0, 0.5), 0.4924773377018894 ],
        [ anarchy.trexpdist(8329801, 0.5), 0.18457219099442898 ],
        [ anarchy.trexpdist(58923, 1.5), 0.2230830365762837 ],
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
        [ anarchy.cohort_outer(-1, 3, 112), 4294967187 ],
    ],
    "cohort_ops": [
        [ anarchy.cohort_interleave(3, 12), 6 ],
        [ anarchy.cohort_interleave(7, 12), 9 ],
    ]
};

let TEST_VALUES = [
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
];

// Testing helpers
let N_SAMPLES = 10000;
let N_CDF_BUCKETS = 100;

// Default tolerance in % of expected value
function tolerance(n_samples) {
    return 1.2/Math.pow(10, Math.log10(n_samples) - 3);
}

function obtain_samples(rng) {
    let result = [];
    for (let i = 0; i < N_SAMPLES; ++i) {
        result.push(rng);
    }
    return result;
}

// Tests whether the mean and standard deviation of some samples is
// within tolerance of the given expected mean and standard
// deviation, and returns the number of checks that failed (0-2). For
// each failed check, a message (which will include the given label)
// will also be pushed onto the given messages array. Pass undefined
// for exp_stdev to skip that check.
function moments_test(samples, exp_mean, exp_stdev, label, messages) {
    let result = 0;
    let tol = tolerance(samples.length);
    let mean = samples.reduce((x, y) => x + y) / samples.length;
    let pct;
    if (exp_mean == 0) {
        pct = Math.abs(mean - exp_mean); // not a real percentage
    } else {
        pct = Math.abs(1 - (mean / exp_mean)); // a percentage
    }
    if (pct > tol) {
        messages.push(
            "Suspicious difference from expected mean ("
            + exp_mean.toFixed(4) + ") for " + label + ": "
            + mean.toFixed(4) + " -> " + (100*pct).toFixed(2) + "% > "
            + (100*tol).toFixed(2) + "%"
        );
        result += 1;
    }
    if (exp_stdev != undefined) {
        let stdev = Math.sqrt(
            samples.map(x => Math.pow(x - mean, 2)).reduce((x, y) => x + y)
            / (samples.length - 1)
        );
        let pct;
        if (exp_stdev == 0) {
            pct = Math.abs(stdev - exp_stdev); // not a real percentage
        } else {
            pct = Math.abs(1 - (stdev / exp_stdev)); // a percentage
        }
        if (pct > tol) {
            messages.push(
                "Suspicious difference expected stdev ("
                + exp_stdev.toFixed(4) + ") for " + label + ": "
                + stdev.toFixed(4) + " -> " + (100*pct).toFixed(2) + "% > "
                + (100*tol).toFixed(2) + "%"
            );
            result += 1;
        }
    }
    return result;
}

// Generates an array of even test points for cdf_test between the
// given lower and upper bounds.
function even_cdf_test_points(lower, upper) {
    let result = [];
    for (let i = 0; i < N_CDF_BUCKETS; ++i) {
        result.push(lower + (i/N_CDF_BUCKETS) * (upper - lower));
    }
    result.push(upper);
    return result;
}

// Computes the area of a trapezoid with the given height, bottom
// length, and top length.
function trapezoid_area(height, top, bottom) {
    return (
        // triangle based on longer - shorter of the top/bottom
        0.5 * Math.abs(top - bottom) * height
        // plus parallelogram based on shorter edge
        + Math.min(top, bottom) * height
    );
}

// Tests that the cumulative distribution function of the given
// samples roughly matches the given expected cumulative distribution
// function (should be a function which accepts a number x and
// returns a probability of the result being smaller than x). This
// estimates the total area of the differences between the actual and
// expected CDFs sampling at each of the given test points, and then
// requires that the percentage that this represents of the area
// under the true CDF is below a certain threshold based on the
// number of samples. This function returns the number of failed
// tests (0 or 1) and if the test fails, it pushes a message (which
// includes the given label) onto the given messages array.
//
// Note: as a side-effect, this function sorts the samples given to
// it.
function cdf_test(samples, cdf, test_points, label, messages) {
    let result = 0;
    let ns = samples.length;
    let tol = tolerance(samples.length);
    samples.sort((a, b) => a - b);
    let exp_precount = cdf(test_points[0]) * ns;
    let obs_precount = 0;
    while (obs_precount < ns && samples[obs_precount] < test_points[0]) {
        obs_precount += 1;
    }
    let prev_exp_pc = exp_precount;
    let prev_obs_pc = obs_precount;
    let prev_overshoot = obs_precount - exp_precount;
    let overshoot;
    let discrepancy_area = 0;
    let correct_area = 0;
    for (let i = 1; i < test_points.length; ++i) {
        exp_precount = cdf(test_points[i]) * ns;
        while (
            obs_precount < ns
            && samples[obs_precount] < test_points[i]
        ) {
            obs_precount += 1;
        }
        // Compute overshoot at this test point
        overshoot = obs_precount - exp_precount;
        // compute top and bottom of (possibly twisted) trapezoid
        let width = test_points[i] - test_points[i-1];
        if (prev_overshoot > 0 == overshoot > 0) {
            // it's a trapezoid; both sides either over- or
            // under-shot.
            discrepancy_area += trapezoid_area(
                width,
                Math.abs(prev_overshoot),
                Math.abs(overshoot)
            );
            if (discrepancy_area < 0) {
                console.warn("D<0 t ", width, prev_overshoot, overshoot);
            }
        } else {
            // it's two triangles; one side did the opposite of the
            // other.
            let inflection;
            if (overshoot != 0) {
                let ratio = Math.abs(prev_overshoot / overshoot);
                inflection = width * ratio / (1 + ratio);
            } else {
                inflection = width;
            }
            discrepancy_area += (
                // triangle from prev test point to inflection point
                0.5 * Math.abs(prev_overshoot) * inflection
                // triangle from inflection point to current test point
                + 0.5 * Math.abs(overshoot) * (width - inflection)
            );
            if (discrepancy_area < 0) {
                console.warn("D<0 x ", prev_overshoot, overshoot, inflection);
            }
        }

        // update correct area
        correct_area += trapezoid_area(width, prev_exp_pc, exp_precount);
        if (correct_area < 0) {
            console.warn("CA<0", width, prev_exp_pc, exp_precount);
        }

        // Update previous letiables for our next step
        prev_exp_pc = exp_precount;
        prev_obs_pc = obs_precount;
        prev_overshoot = overshoot;
    }

    let discrepancy = Math.abs(discrepancy_area / correct_area);
    if (discrepancy > tol) {
        messages.push(
            "Suspicious CDF area difference from "
            + correct_area.toFixed(2) + " for " + label + ": "
            + discrepancy_area.toFixed(2) + " -> "
            + (100 * discrepancy).toFixed(2) + "% > "
            + (100 * tol).toFixed(2) + "%"
        );
        result += 1;
    }
    return result;
}

let EXEC_TESTS = {
    "unit_ops": function () {
        let result = 0;
        let messages = [];
        for (let o = 0; o < 10; ++o) {
            for (let i = 0; i < 1024*32; ++i) {
                let x = (i << o);
                if (x != anarchy.flop(anarchy.flop(x))) {
                    messages.push("flop @ " + x);
                    result += 1;
                }
                if (x != anarchy.rev_scramble(anarchy.scramble(x))) {
                    messages.push("scramble @ " + x);
                    result += 1;
                }
                if (x != anarchy.prng(anarchy.rev_prng(x))) {
                    messages.push("prng @ " + x);
                    result += 1;
                }
                let r = x;
                let p = x;
                for (let j = 0; j < TEST_VALUES.length; ++j) {
                    let y = TEST_VALUES[j];
                    if (
                        x != anarchy.rev_swirl(
                            anarchy.swirl(x, y),
                            y
                        )
                    ) {
                        messages.push("swirl @ " + x + ", " + y);
                        result += 1;
                    }
                    if (x != anarchy.fold(anarchy.fold(x, y), y)) {
                        messages.push("fold @ " + x + ", " + y);
                        result += 1;
                    }
                    if (r != anarchy.scramble(anarchy.rev_scramble(r))) {
                        messages.push("chained scramble @ " + r);
                        result += 1;
                    }
                    r = anarchy.rev_scramble(r);
                    if (p != anarchy.rev_prng(anarchy.prng(p))) {
                        messages.push("chained prng @ " + p);
                        result += 1;
                    }
                    p = anarchy.prng(p);
                }
            }
        }
        if (result != 0) {
            console.log("unit_ops failures:");
            messages.forEach(function (m) {
                console.log(m);
            });
        }
        return result;
    },
    "cohort_ops": function () {
        let result = 0;
        let messages = [];
        let cohort_sizes = [ 3, 12, 17, 32, 1024 ];
        for (let idx = 0; idx < cohort_sizes.length; ++idx) {
            let cs = cohort_sizes[idx];
            let observed = {
                "interleave": {},
                "fold": {},
                "spin": {},
                "flop": {},
                "mix": {},
                "spread": {},
                "upend": {},
                "shuffle": {},
            };
            for (let i = 0; i < cs; ++i) {
                // interleave
                let x = anarchy.cohort_interleave(i, cs);
                let rc = anarchy.rev_cohort_interleave(x, cs);
                if (i != rc) {
                    messages.push(
                        "interleave (" + cs + ") @ " + i + " → "
                        + x + " → " + rc
                    );
                    result += 1;
                }
                let v = "" + x;
                if (observed["interleave"].hasOwnProperty(v)) {
                    observed["interleave"][v] += 1;
                } else {
                    observed["interleave"][v] = 1;
                }
            }
            for (let j = 0; j < TEST_VALUES.length; ++j) {
                let seed = TEST_VALUES[j];
                observed["fold"]["" + j] = {};
                observed["spin"]["" + j] = {};
                observed["flop"]["" + j] = {};
                observed["mix"]["" + j] = {};
                observed["spread"]["" + j] = {};
                observed["upend"]["" + j] = {};
                observed["shuffle"]["" + j] = {};
                for (let i = 0; i < cs; ++i) {
                    // fold
                    let x = anarchy.cohort_fold(i, cs, seed);
                    let rc = anarchy.rev_cohort_fold(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "fold (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    let v = "" + x;
                    if (observed["fold"]["" + j].hasOwnProperty(v)) {
                        observed["fold"]["" + j][v] += 1;
                    } else {
                        observed["fold"]["" + j][v] = 1;
                    }
                    // spin
                    x = anarchy.cohort_spin(i, cs, seed);
                    rc = anarchy.rev_cohort_spin(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "spin (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    v = "" + x;
                    if (observed["spin"]["" + j].hasOwnProperty(v)) {
                        observed["spin"]["" + j][v] += 1;
                    } else {
                        observed["spin"]["" + j][v] = 1;
                    }
                    // flop
                    x = anarchy.cohort_flop(i, cs, seed);
                    rc = anarchy.cohort_flop(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "flop (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    v = "" + x;
                    if (observed["flop"]["" + j].hasOwnProperty(v)) {
                        observed["flop"]["" + j][v] += 1;
                    } else {
                        observed["flop"]["" + j][v] = 1;
                    }
                    // mix
                    x = anarchy.cohort_mix(i, cs, seed);
                    rc = anarchy.rev_cohort_mix(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "mix (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    v = "" + x;
                    if (observed["mix"]["" + j].hasOwnProperty(v)) {
                        observed["mix"]["" + j][v] += 1;
                    } else {
                        observed["mix"]["" + j][v] = 1;
                    }
                    // spread
                    x = anarchy.cohort_spread(i, cs, seed);
                    rc = anarchy.rev_cohort_spread(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "spread (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    v = "" + x;
                    if (observed["spread"]["" + j].hasOwnProperty(v)) {
                        observed["spread"]["" + j][v] += 1;
                    } else {
                        observed["spread"]["" + j][v] = 1;
                    }
                    // upend
                    x = anarchy.cohort_upend(i, cs, seed);
                    rc = anarchy.cohort_upend(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "upend (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    v = "" + x;
                    if (observed["upend"]["" + j].hasOwnProperty(v)) {
                        observed["upend"]["" + j][v] += 1;
                    } else {
                        observed["upend"]["" + j][v] = 1;
                    }
                    // shuffle
                    x = anarchy.cohort_shuffle(i, cs, seed);
                    rc = anarchy.rev_cohort_shuffle(x, cs, seed);
                    if (i != rc) {
                        messages.push(
                            "shuffle (" + cs + ")@ " + i + " / " + seed
                            + " → " + x + " → " + rc
                        );
                        result += 1;
                    }
                    v = "" + x;
                    if (observed["shuffle"]["" + j].hasOwnProperty(v)) {
                        observed["shuffle"]["" + j][v] += 1;
                    } else {
                        observed["shuffle"]["" + j][v] = 1;
                    }
                }
            }
            for (let prp in observed) {
                if (observed.hasOwnProperty(prp)) {
                    if (prp == "interleave") {
                        for (let i = 0; i < cs; ++i) {
                            let v = "" + i;
                            let count = observed[prp][v];
                            if (count == undefined) {
                                count = 0;
                            }
                            if (count != 1) {
                                messages.push(
                                    prp + " (" + cs + ") @ " + i + " found "
                                    + count
                                );
                                result += 1;
                            }
                        }
                    } else {
                        for (let j = 0; j < TEST_VALUES.length; ++j) {
                            let seed = TEST_VALUES[j];
                            let k = "" + j;
                            for (let i = 0; i < cs; ++i) {
                                let v = "" + i;
                                let count = observed[prp][k][v];
                                if (count == undefined) {
                                    count = 0;
                                }
                                if (count != 1) {
                                    messages.push(
                                        prp + " (" + cs + ") @ " + i + " / "
                                        + seed + " found " + count
                                    );
                                    result += 1;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (result != 0) {
            console.log("cohort_ops failures:");
            messages.forEach(function (m) {
                console.log('  ' + m);
            });
        }
        return result;
    },

    "distribution_functions": function () {
        let result = 0;
        let messages = [];

        // Tests of udist
        for (let seed of TEST_VALUES) {
            let rng = seed;
            let samples = [];

            // create samples
            for (let i = 0; i < N_SAMPLES; ++i) {
                samples.push(anarchy.udist(rng));
                rng = anarchy.prng(rng, seed);
            }

            // test mean and standard deviation
            result += moments_test(
                samples,
                0.5,
                1/Math.sqrt(12),
                "udist(:" + seed + ":)", 
                messages
            );

            // test CDF
            result += cdf_test(
                samples,
                x => x,
                even_cdf_test_points(0, 1),
                "udist(:" + seed + ":)", 
                messages
            );
        }

        // Tests of pgdist
        for (let seed of TEST_VALUES) {
            let rng = seed;
            let samples = [];

            let exp_mean = 0.5;
            // The expected standard deviation for the average of three
            // uniformly distributed letiables (we know pgdist uses 3
            // samples) is computed as follows:
            //
            // 1. The letiance of the sum of several independent
            //    letiables is the sum of their letiances; we know the
            //    letiance of the uniform distribution is 1/12, so this
            //    is 3/12, or 1/4 for the sum of the three samples.
            // 2. The letiance of a distribution multiplied by a constant
            //    is the letiance of the original distribution multiplied
            //    by the square of that constant. So when we divide by 3
            //    to compute the average of the three samples, we're
            //    multiplying by 1/3 and so the letiance needs to be
            //    multiplied by 1/9. This gives us 1/36 as the letiance
            //    of the average.
            // 3. The standard deviation is the square root of the
            //    letiance, so we get 1/6 as the expected standard
            //    deviation of the average of three uniformly distributed
            //    letiables on [0, 1).
            let exp_stdev = 1/6;

            // compute samples
            for (let i = 0; i < N_SAMPLES; ++i) {
                samples.push(anarchy.pgdist(rng));
                rng = anarchy.prng(rng, seed);
            }

            // test mean and standard deviation
            result += moments_test(
                samples,
                exp_mean,
                exp_stdev,
                "pgdist(:" + seed + ":)", 
                messages
            );

            // test CDF
            // TODO: Test the CDF?
            /*
               result += cdf_test(
               samples,
               ???,
               even_cdf_test_points(0, 1),
               "pgdist(" + seed + ")", 
               messages
               );
               */
        }

        // Tests of flip
        for (let seed of TEST_VALUES) {
            // Test with different probabilities for true
            for (let p of [0.02, 0.2, 0.5, 0.9, 0.99]) {
                let rng = seed;
                let samples = [];
                let exp_mean = p;
                let exp_stdev = Math.sqrt(
                    (
                        ((1000 * p) * Math.pow(1 - p, 2))
                        + ((1000 * (1-p)) * Math.pow(p, 2))
                    ) / 999
                );

                // compute samples
                for (let i = 0; i < N_SAMPLES; ++i) {
                    let x = anarchy.flip(p, rng);
                    if (typeof x != "boolean") {
                        result += 1;
                        messages.push(
                            "non-boolean flip result for probability "
                            + p + " with seed " + seed + ": " + x
                        );
                    }
                    samples.push(+x);
                    rng = anarchy.prng(rng, seed);
                }

                // Test moments
                result += moments_test(
                    samples,
                    exp_mean,
                    exp_stdev,
                    "flip(" + p + ", :" + seed + ":)", 
                    messages
                );
            }
        }

        // Tests of idist
        for (let seed of TEST_VALUES) {
            // Test with different min/max values
            for (let low of [0, -31, 1289482, -7298392]) {
                for (let high of [0, -31, 1289482, -7298392]) {
                    let rng = seed;
                    let samples = [];

                    // compute samples
                    for (let i = 0; i < N_SAMPLES; ++i) {
                        samples.push(anarchy.idist(rng, low, high));
                        rng = anarchy.prng(rng, seed);
                    }

                    // expected average and stdev
                    let exp_mean = (low + (high-1))/2;
                    let range = high - 1 - low;
                    // scaled from uniform distribution stdev:
                    let exp_stdev = Math.sqrt(1/12) * Math.abs(range);

                    // Special case for constant results
                    if (Math.abs(high - low) <= 1) {
                        exp_mean = low;
                        exp_stdev = 0;
                    }

                    result += moments_test(
                        samples,
                        exp_mean,
                        exp_stdev,
                        "idist(:" + seed + ":, " + low + ", " + high + ")",
                        messages
                    );

                    let real_low = Math.min(low, high);
                    let real_high = Math.max(low, high);
                    if (real_high > real_low + 1) {
                        result += cdf_test(
                            samples,
                            /* jshint -W083 */
                            x => (x - real_low) / (real_high - real_low),
                            /* jshint +W083 */
                            even_cdf_test_points(real_low, real_high),
                            "idist(:" + seed + ":, " + low + ", " + high + ")",
                            messages
                        );
                    }
                }
            }
        }

        // Tests of expdist
        for (let seed of TEST_VALUES) {
            for (let lambda of [0.05, 0.5, 1, 1.5, 5]) {
                // Test with different lambda values
                let rng = seed;
                let samples = [];

                let exp_mean = 1/lambda;
                // expected stdev (same as expected mean for an exponential
                // distribution, according to Wikipedia)
                let exp_stdev = exp_mean;

                // compute samples
                for (let i = 0; i < N_SAMPLES; ++i) {
                    samples.push(anarchy.expdist(rng, lambda));
                    rng = anarchy.prng(rng, seed);
                }

                result += moments_test(
                    samples,
                    exp_mean,
                    exp_stdev,
                    "expdist(:" + seed + ":, " + lambda + ")", 
                    messages
                );

                result += cdf_test(
                    samples,
                    /* jshint -W083 */
                    x => 1 - Math.exp(-lambda * x),
                    /* jshint +W083 */
                    Array.prototype.concat(
                        even_cdf_test_points(0, 2),
                        even_cdf_test_points(2.5, 30)
                    ),
                    "expdist(:" + seed + ":, " + lambda + ")", 
                    messages
                );
            }
        }

        // Tests of trexpdist
        for (let seed of TEST_VALUES) {
            for (let lambda of [0.05, 0.5, 1, 1.5, 5]) {
                // Test with different lambda values
                let rng = seed;
                let samples = [];

                // See http://docsdrive.com/pdfs/sciencepublications/jmssp/2008/284-288.pdf
                let exp_mean = 1/lambda - 1/(Math.exp(lambda) - 1);
                // TODO: What's the expected stdev here?
                let exp_stdev = undefined;

                // compute samples
                for (let i = 0; i < N_SAMPLES; ++i) {
                    samples.push(anarchy.trexpdist(rng, lambda));
                    rng = anarchy.prng(rng, seed);
                }

                result += moments_test(
                    samples,
                    exp_mean,
                    exp_stdev,
                    "trexpdist(:" + seed + ":, " + lambda + ")", 
                    messages
                );

                // Per https://en.wikipedia.org/wiki/Truncated_distribution:
                // truncated PDF is derived from original PDF and CDF using
                //   f(x) = g(x) / (F(b) - F(a))
                //
                // For our truncated exponential distribution, a is 0 and
                // b is 1, so F(b) - F(a) is 
                //
                //  divisor = 1 - e^(-lambda)
                //
                //  (which is just the CDF of the exponential
                //  distribution at x=1)
                //
                // The truncated CDF is the integral of the truncated
                // PDF:
                //
                //   F(x) = \int_0^x{g(x) / divisor
                //
                // Since g(x) is the original PDF on the range of that
                // integral, we can use
                //
                //   g(x) = lambda * e^(-lambda*x)
                //
                // The indefinite integral of g(x) is
                //
                //   -e^(-lambda*x)
                //
                // So we can use -e^(-lambda*x) + e^(-lambda*0) as the
                // definite integral from 0 to 1, which simplifies to:
                //
                //   numerator = 1 - e^(-lambda*x)
                //
                // Essentially, we're just scaling up the old CDF
                // slightly by a constant factor so that it reaches 1
                // when x=1 instead of reaching 1 at infinity.
                /* jshint -W083 */
                let truncated_cdf = function(x) {
                    let divisor = 1 - Math.exp(-lambda);
                    let numerator = 1 - Math.exp(-lambda * x);
                    return numerator / divisor;
                };
                /* jshint +W083 */

                result += cdf_test(
                    samples,
                    truncated_cdf,
                    even_cdf_test_points(0, 1),
                    "trexpdist(:" + seed + ":, " + lambda + ")", 
                    messages
                );
            }
        }

        if (result != 0) {
            console.log("distribution_functions failures:");
            messages.forEach(function (m) {
                console.log('  ' + m);
            });
        }
        return result;
    }
};

function run_value_tests() {
    display_message("Starting value tests...");
    for (let t in VALUE_TESTS) {
        if (VALUE_TESTS.hasOwnProperty(t)) {
            let test_count = VALUE_TESTS[t].length;
            let passed = 0;
            for (let index = 0; index < VALUE_TESTS[t].length; ++index) {
                let sub_t = VALUE_TESTS[t][index];
                if (sub_t[0] == sub_t[1]) {
                    passed += 1;
                } else {
                    display_message("Test failed: " + t + "." + index);
                    display_message(
                        "&nbsp;&nbsp;expected: " + sub_t[1] + " got: "
                        + sub_t[0]
                    );
                }
            }
            display_message(
                "Suite '" + t + "': passed " + passed + " / " + test_count
            );
        }
    }
    display_message("Done with value tests.");
}

function run_exec_tests() {
    for (let t in EXEC_TESTS) {
        if (EXEC_TESTS.hasOwnProperty(t)) {
            let result = EXEC_TESTS[t]();
            if (result != 0) {
                display_message(
                    "Test '" + t + "' failed " + result +" sub-tests."
                );
            } else {
                display_message("Test '" + t + "' succeeded.");
            }
        }
    }
}

// do it!
run_value_tests();
run_exec_tests();
