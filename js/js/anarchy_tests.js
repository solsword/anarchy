requirejs.config({
  baseURL: "js/",
});

// anarchy_tests.js
// Tests for the anarchy reversible chaos library.


requirejs(
  ["anarchy"],
  function(anarchy) {
    function display_message(m) {
      document.body.innerHTML += "<div>" + m + "</div>";
    }

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
        [ anarchy.udist(0), 0 ],
        [ anarchy.udist(8329801), 0.10328831051654581 ],
        [ anarchy.udist(58923), 0.9000991587987678 ],
      ],
      "idist": [
        // TODO: Verify and/or fix these!
        [ anarchy.idist(0, 0, 1), 0 ],
        [ anarchy.idist(0, 3, 25), 3 ],
        [ anarchy.idist(58923, 3, 25), 22 ],
        [ anarchy.idist(58923, -2, -4), -4 ],
      ],
      "expdist": [
        // TODO: Verify and/or fix these!
        [ anarchy.expdist(0), 0 ], // TODO: Um...?
        [ anarchy.expdist(8329801), 0.2180417699901415 ],
        [ anarchy.expdist(58923), 4.607154345860652 ],
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
    ];

    EXEC_TESTS = {
      "unit_ops": function () {
        var result = 0;
        var messages = [];
        for (var o = 0; o < 10; ++o) {
          for (var i = 0; i < 1024*32; ++i) {
            var x = (i << o);
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
            var r = x;
            var p = x;
            for (var j = 0; j < TEST_VALUES.length; ++j) {
              var y = TEST_VALUES[j];
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
          console.log("unit_ops failures:")
          messages.forEach(function (m) {
            console.log(m);
          });
        }
        return result;
      },
      "cohort_ops": function () {
        var result = 0;
        var messages = [];
        var cohort_sizes = [ 3, 12, 17, 32, 1024 ];
        for (var idx = 0; idx < cohort_sizes.length; ++idx) {
          var cs = cohort_sizes[idx];
          var observed = {
            "interleave": {},
            "fold": {},
            "spin": {},
            "flop": {},
            "mix": {},
            "spread": {},
            "upend": {},
            "shuffle": {},
          };
          for (var i = 0; i < cs; ++i) {
            // interleave
            var x = anarchy.cohort_interleave(i, cs);
            var rc = anarchy.rev_cohort_interleave(x, cs);
            if (i != rc) {
              messages.push(
                "interleave (" + cs + ") @ " + i + " → " + x + " → " + rc
              )
              result += 1;
            }
            var v = "" + x;
            if (observed["interleave"].hasOwnProperty(v)) {
              observed["interleave"][v] += 1;
            } else {
              observed["interleave"][v] = 1;
            }
          }
          for (var j = 0; j < TEST_VALUES.length; ++j) {
            var seed = TEST_VALUES[j];
            observed["fold"]["" + j] = {};
            observed["spin"]["" + j] = {};
            observed["flop"]["" + j] = {};
            observed["mix"]["" + j] = {};
            observed["spread"]["" + j] = {};
            observed["upend"]["" + j] = {};
            observed["shuffle"]["" + j] = {};
            for (var i = 0; i < cs; ++i) {
              // fold
              var x = anarchy.cohort_fold(i, cs, seed);
              var rc = anarchy.rev_cohort_fold(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "fold (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["fold"]["" + j].hasOwnProperty(v)) {
                observed["fold"]["" + j][v] += 1;
              } else {
                observed["fold"]["" + j][v] = 1;
              }
              // spin
              var x = anarchy.cohort_spin(i, cs, seed);
              var rc = anarchy.rev_cohort_spin(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "spin (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["spin"]["" + j].hasOwnProperty(v)) {
                observed["spin"]["" + j][v] += 1;
              } else {
                observed["spin"]["" + j][v] = 1;
              }
              // flop
              var x = anarchy.cohort_flop(i, cs, seed);
              var rc = anarchy.cohort_flop(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "flop (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["flop"]["" + j].hasOwnProperty(v)) {
                observed["flop"]["" + j][v] += 1;
              } else {
                observed["flop"]["" + j][v] = 1;
              }
              // mix
              var x = anarchy.cohort_mix(i, cs, seed);
              var rc = anarchy.rev_cohort_mix(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "mix (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["mix"]["" + j].hasOwnProperty(v)) {
                observed["mix"]["" + j][v] += 1;
              } else {
                observed["mix"]["" + j][v] = 1;
              }
              // spread
              var x = anarchy.cohort_spread(i, cs, seed);
              var rc = anarchy.rev_cohort_spread(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "spread (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["spread"]["" + j].hasOwnProperty(v)) {
                observed["spread"]["" + j][v] += 1;
              } else {
                observed["spread"]["" + j][v] = 1;
              }
              // upend
              var x = anarchy.cohort_upend(i, cs, seed);
              var rc = anarchy.cohort_upend(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "upend (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["upend"]["" + j].hasOwnProperty(v)) {
                observed["upend"]["" + j][v] += 1;
              } else {
                observed["upend"]["" + j][v] = 1;
              }
              // shuffle
              var x = anarchy.cohort_shuffle(i, cs, seed);
              var rc = anarchy.rev_cohort_shuffle(x, cs, seed);
              if (i != rc) {
                messages.push(
                  "shuffle (" + cs + ")@ " + i + " / " + seed
                + " → " + x + " → " + rc
                )
                result += 1;
              }
              var v = "" + x;
              if (observed["shuffle"]["" + j].hasOwnProperty(v)) {
                observed["shuffle"]["" + j][v] += 1;
              } else {
                observed["shuffle"]["" + j][v] = 1;
              }
            }
          }
          for (var prp in observed) {
            if (observed.hasOwnProperty(prp)) {
              if (prp == "interleave") {
                for (var i = 0; i < cs; ++i) {
                  var v = "" + i;
                  var count = observed[prp][v];
                  if (count == undefined) {
                    count = 0;
                  }
                  if (count != 1) {
                    messages.push(
                      prp + " (" + cs + ") @ " + i + " found " + count
                    );
                    result += 1;
                  }
                }
              } else {
                for (var j = 0; j < TEST_VALUES.length; ++j) {
                  var seed = TEST_VALUES[j];
                  var k = "" + j;
                  for (var i = 0; i < cs; ++i) {
                    var v = "" + i;
                    var count = observed[prp][k][v];
                    if (count == undefined) {
                      count = 0;
                    }
                    if (count != 1) {
                      messages.push(
                        prp + " (" + cs + ") @ " + i + " / " + seed
                      + " found " + count
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
          console.log("cohort_ops failures:")
          messages.forEach(function (m) {
            console.log('  ' + m);
          });
        }
        return result;
      },

      "distribution_functions": function () {
        let result = 0;
        let messages = [];

        let n_trials = 100000;
        let n_buckets = 1000;

        // Tests of udist
        for (let seed of TEST_VALUES) {
            let rng = seed;
            let avg = 0;
            let samples = [];
            let buckets = [];

            // compute average and count # in each bucket
            for (let i = 0; i < n_trials; ++i) {
                let x = anarchy.udist(rng);
                samples.push(x);
                avg += x;
                let bi = Math.floor(x * n_buckets);
                if (buckets[bi] != undefined) {
                    buckets[bi] += 1;
                } else {
                    buckets[bi] = 1;
                }
                rng = anarchy.prng(rng, seed);
            }
            avg /= n_trials;

            // compute stdev
            let stdev = 0;
            for (let i = 0; i < n_trials; ++i) {
                stdev += Math.pow(samples[i] - avg, 2);
            }
            stdev /= (n_trials - 1);
            stdev = Math.sqrt(stdev);

            // compute average difference in bucket counts
            let avg_count_diff = 0;
            for (let i = 0; i < n_buckets; ++i) {
                let diff = Math.abs(buckets[i] - (n_trials / n_buckets));
                avg_count_diff += diff;
            }
            avg_count_diff /= n_buckets;

            let tolerance = 1/(10 * (Math.log10(n_trials) - 3));
            if (Math.abs(avg - 0.5) > tolerance) {
                result += 1;
                messages.push(
                    "Distressingly large udist mean difference from 0.5"
                  + " for seed " + seed + ": "
                  + avg
                );
            }

            if (Math.abs(stdev - Math.sqrt(1/12)) > tolerance) {
                result += 1;
                messages.push(
                    "Distressingly large udist stdev difference from 1/√12 "
                  + "(0.288675) for seed " + seed + ": "
                  + stdev
                );
            }

            if (avg_count_diff > (n_trials / n_buckets)/2) {
                result += 1;
                messages.push(
                    "Distressingly large udist bucket difference average "
                  + " for seed " + seed + ": "
                  + avg_count_diff
                );
            }
        }

        // Tests of pgdist
        for (let seed of TEST_VALUES) {
            let rng = seed;
            let avg = 0;
            let samples = [];

            // compute average and count # in each bucket
            for (let i = 0; i < n_trials; ++i) {
                let x = anarchy.pgdist(rng);
                samples.push(x);
                avg += x;
                rng = anarchy.prng(rng, seed);
            }
            avg /= n_trials;

            // compute stdev
            let stdev = 0;
            for (let i = 0; i < n_trials; ++i) {
                stdev += Math.pow(samples[i] - avg, 2);
            }
            stdev /= (n_trials - 1);
            stdev = Math.sqrt(stdev);

            let tolerance = 1/(10 * (Math.log10(n_trials) - 3));
            if (Math.abs(avg - 0.5) > tolerance) {
                result += 1;
                messages.push(
                    "Distressingly large pgdist mean difference from 0.5"
                  + " for seed " + seed + ": "
                  + avg
                );
            }

            // TODO: Derive a correct expected value here!
            if (Math.abs(stdev - 0.1588) > tolerance) {
                result += 1;
                messages.push(
                    "Distressingly large pgdist stdev difference from "
                  + "(0.1588) for seed " + seed + ": "
                  + stdev
                );
            }
        }

        // Tests of flip
        for (let seed of TEST_VALUES) {
            // Test with different probabilities for true
            for (let p of [0.02, 0.2, 0.5, 0.9, 0.99]) {
                let rng = seed;
                let avg = 0;
                let samples = [];
                let exp_mean = p;
                let exp_stdev = Math.sqrt(
                    (
                        ((1000 * p) * Math.pow(p, 2))
                      + ((1000 * (1-p)) * Math.pow(1 - p, 2))
                    ) / 999
                );
                // TODO: Correct values here

                // compute average and count # in each bucket
                for (let i = 0; i < n_trials; ++i) {
                    let x = anarchy.flip(rng); // 0 or 1 instead of true/false
                    if (typeof x != "boolean") {
                        result += 1;
                        messages.push(
                            "non-boolean flip result for probability "
                          + p + " with seed " + seed + ": " + x
                        );
                    }
                    samples.push(+x);
                    avg += +x;
                    rng = anarchy.prng(rng, seed);
                }
                avg /= n_trials;

                // compute stdev
                let stdev = 0;
                for (let i = 0; i < n_trials; ++i) {
                    stdev += Math.pow(samples[i] - avg, 2);
                }
                stdev /= (n_trials - 1);
                stdev = Math.sqrt(stdev);

                let tolerance = 1/(10 * (Math.log10(n_trials) - 3));
                if (Math.abs(avg - exp_mean) > tolerance) {
                    result += 1;
                    messages.push(
                        "Distressingly large flip mean difference from "
                      + exp_mean + " for seed " + seed + ": "
                      + avg
                    );
                }

                if (Math.abs(stdev - exp_stdev) > tolerance) {
                    result += 1;
                    messages.push(
                        "Distressingly large flip stdev difference from "
                      + "" + exp_stdev + " for seed " + seed + ": "
                      + stdev
                    );
                }
            }
        }

        if (result != 0) {
          console.log("distribution_functions failures:")
          messages.forEach(function (m) {
            console.log('  ' + m);
          });
        }
        return result;
      }
    }

    function run_value_tests() {
      display_message("Starting value tests...");
      for (var t in VALUE_TESTS) {
        if (VALUE_TESTS.hasOwnProperty(t)) {
          var test_count = VALUE_TESTS[t].length;
          var passed = 0;
          VALUE_TESTS[t].forEach(function (sub_t, index) {
            if (sub_t[0] == sub_t[1]) {
              passed += 1;
            } else {
              display_message("Test failed: " + t + "." + index);
              display_message(
                "&nbsp;&nbsp;expected: " + sub_t[1] + " got: " + sub_t[0]
              );
            }
          });
          display_message(
            "Suite '" + t + "': passed " + passed + " / " + test_count
          );
        }
      }
      display_message("Done with value tests.");
    }

    function run_exec_tests() {
      for (var t in EXEC_TESTS) {
        if (EXEC_TESTS.hasOwnProperty(t)) {
          var result = EXEC_TESTS[t]()
          if (result != 0) {
            display_message("Test '" + t + "' failed " + result +" sub-tests.");
          } else {
            display_message("Test '" + t + "' succeeded.")
          }
        }
      }
    }

    // do it!
    run_value_tests();
    run_exec_tests();
  }
);
