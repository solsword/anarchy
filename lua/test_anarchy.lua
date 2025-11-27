-- test_anarchy.lua
-- Testing script for anarchy reversible chaos module.
-- Adapted from Python version testing script.
-- Note: This script has been tested using Lua compiled with 64-bit
-- integers. It will probably not work completely with 32-bit integers.
-- If you're for some reason interested in helping make it work with
-- 32-bit integers, feel free to reach out.

local anarchy = require("anarchy")
local util = anarchy.util
local rng = anarchy.rng
local cohort = anarchy.cohort
local distribution = anarchy.distribution

local floor = math.floor
local min = math.min
local max = math.max
local exp = math.exp
local log10 = math.log10
local abs = math.abs


--===================--
-- Setup & Constants --
--===================--

local TEST_VALUES = {
    0,
    1,
    3,
    17,
    48,
    64,
    1029,
    8510938,
    1928301928,
    0x80000000  -- 1 << 31
}
-- More test values if we have 64 bits
if anarchy.ID_BYTES == 8 then
    TEST_VALUES[#TEST_VALUES + 1] = 13834298198122839
    TEST_VALUES[#TEST_VALUES + 1] = 0x8000000000000000  -- 1 << 63
    TEST_VALUES[#TEST_VALUES + 1] = 0x0f0003000e007020
    TEST_VALUES[#TEST_VALUES + 1] = 0x0000000100000000  -- 1 << 32
end

local SEQ_SEEDS = {}
for i = 1,10000 do
    SEQ_SEEDS[i] = i
end

local N_SAMPLES = 25000
local N_CDF_BUCKETS = 100

local PROGRESS = 0
local PROGRESS_LABEL = "Progress"
local TOTAL_WORK = nil


function reset_progress()
    -- Resets the progress counter.
    PROGRESS = 0
end


function start_progress(label, work_units)
    -- Sets up for progress printing by declaring the label and the total
    -- units of work to be accomplished. Sets PROGRESS to 0 as well.
    PROGRESS = 0
    PROGRESS_LABEL = label
    TOTAL_WORK = work_units
end


function progress()
    -- Increments the progress counter and prints a message if that
    -- increment achieved 1% of progress.
    if PROGRESS == 0 then
        io.write("\n" .. PROGRESS_LABEL .. ": 0%\r")
        io.flush()
    elseif PROGRESS == TOTAL_WORK - 1 then
        io.write(PROGRESS_LABEL .. ": 100%\n")  -- now we introduce a newline
        io.flush()
    elseif TOTAL_WORK ~= nil then
        local old_pct = floor((PROGRESS * 100) / TOTAL_WORK)
        local new_pct = floor(((PROGRESS + 1) * 100) / TOTAL_WORK)
        if old_pct ~= new_pct then
            io.write(PROGRESS_LABEL .. ": " .. tostring(new_pct) .. "%\r")
            io.flush()
        end
    end
    PROGRESS = PROGRESS + 1
end


function tolerance(n_samples)
    -- Computes a tolerance value based on a number of samples for testing
    -- pseudo-random functions.
    -- 1.5 instead of 1 here nudges things to be a bit more
    -- permissive...
    return 1.5 / (10 ^ max(-1, (log10(n_samples) - 3)))
end


function binomial_tolerance(n_samples)
    -- Like tolerance but more permissive for binomial distributions...
    -- TODO: Why do I even need this? T_T
    return 1.6 / (10 ^ max(-1, (log10(n_samples) - 3.5)))
end


function moments_test(
    samples,
    exp_mean,
    exp_stdev,
    label,
    tol
)
    -- Tests the given list of samples to make sure it has close to the given
    -- expected mean and standard deviation. Uses assertions to indicate
    -- success/failure. The exp_stdev can be given as None to skip that
    -- test.
    --
    -- If no tolerance value is given, a tolerance will be computed
    -- automatically using the tolerance function.
    --
    -- The label is used in failure messages.
    if tol == nil then
        tol = tolerance(#samples)
    end
    local mean = 0
    for _, s in ipairs(samples) do
        mean = mean + s
    end
    mean = mean / #samples

    local pct
    if exp_mean == 0 then
        pct = abs(mean)  -- not a percentage
    else
        pct = abs((mean / exp_mean) - 1)  -- a percentage
    end

    assert(pct <= tol, (
        string.format(
            (
                "Suspicious mean discrepancy from (%.4f) for"
             .. " %s: %.4f -> %.2f%%"
            ),
            exp_mean, label, mean, 100 * pct
        )
    ))

    if exp_stdev ~= nil then
        local stdev = 0
        for _, s in ipairs(samples) do
            stdev = stdev + (s - mean) ^ 2
        end

        stdev = stdev / (#samples - 1)
        stdev = stdev ^ 0.5

        if exp_stdev == 0 then
            pct = stdev  -- not a percentage
        else
            pct = abs(1 - stdev / exp_stdev)  -- actually a percentage
        end

        assert(pct <= tol, (
            string.format(
                (
                    "Suspicious stdev discrepancy from (%.4f) for"
                 .. " %s: %.4f -> %.2f%% > %.2f%%"
                ),
                exp_stdev, label, stdev, 100 * pct, 100 * tol
            )
        ))
    end
end


function binomial_samples_test(
    samples,
    p,
    n_groups,
    label,
    tol
)
    -- Tests the given list of samples from a binomial distribution (each
    -- sample should be 'true' or 'false') to make sure it has close to
    -- the correct mean number of successes and standard deviation of
    -- successes per group when split into the given number of groups.
    -- Uses assertions to indicate success/failure.
    -- If no tolerance value is given, a tolerance will be computed
    -- automatically using the tolerance function. The label is used in
    -- failure messages.
    local mean_tol, stdev_tol
    if tol == nil then
        mean_tol = binomial_tolerance(#samples)
    else
        mean_tol = tol
    end
    -- TODO: Allow specifying this?
    stdev_tol = tolerance(n_groups)

    local count = 0
    for _, s in ipairs(samples) do
        if s then
            count = count + 1
        end
    end

    local exp_count = #samples * p

    local pct
    if exp_count == 0 then
        pct = abs(count)  -- not a percentage
    else
        pct = abs((count / exp_count) - 1)  -- a percentage
    end

    assert(pct <= mean_tol, (
        string.format(
            (
                "Suspicious count discrepancy from (%.4f) for"
             .. " %s: %.4f -> %.2f%%, > %.2f%%"
            ),
            exp_count, label, count, 100 * pct, 100 * mean_tol
        )
    ))

    local gsize = floor(#samples / n_groups)
    local gcounts = {}
    local mean_gc = 0
    for g = 1, n_groups do
        local gcount = 0
        for i = 1, gsize do
            local ii = gsize * (g - 1) + i
            if samples[ii] then
                gcount = gcount + 1
            end
        end
        mean_gc = mean_gc + gcount
        gcounts[#gcounts + 1] = gcount
    end
    mean_gc = mean_gc / n_groups

    -- Compute actual standard deviation of the counts within each group
    local stdev = 0
    for _, gc in ipairs(gcounts) do
        stdev = stdev + (gc - mean_gc) ^ 2
    end

    stdev = (stdev / (n_groups - 1)) ^ 0.5

    -- Compute expected standard deviation
    local exp_stdev = (gsize * p * (1 - p)) ^ 0.5

    if exp_stdev == 0 then
        pct = stdev  -- not a percentage
    else
        pct = abs((stdev / exp_stdev) - 1)  -- actually a percentage
    end

    assert(pct <= stdev_tol, (
        string.format(
            (
                "Suspicious groups stdev discrepancy from (%.4f) for"
             .. " %s: %.4f -> %.2f%% > %.2f%%"
            ),
            exp_stdev, label, stdev, 100 * pct, 100 * stdev_tol
        )
    ))
end


-- How many bins to use when computing subsamples for extended moments
-- tests:
local MOMENT_SUBSAMPLES_BINS = 100


function moments_and_subsamples_test(
    samples,
    exp_mean,
    exp_stdev,
    label,
    tol
)
    -- Tests the overall mean and standard deviation of all the given
    -- samples, and then tests the same thing for
    -- `MOMENT_SUBSAMPLES_BINS` sub-samples dividing along the samples
    -- index and for the same numer sub-samples dividing by index modulo
    -- that number. Also checks for directional bias in the errors among
    -- the subsamples.
    --
    -- Uses automatic tolerance when checking subsamples.
    --
    -- We assume that the distribution is a normal distribution when
    -- scaling the expected standard deviation for the subsamples...
    -- TODO: Not that!
    moments_test(samples, exp_mean, exp_stdev, label, tol)

    local subsample_exp_stdev = nil
    if exp_stdev ~= nil then
        -- Compute expected subsample standard deviation based on
        -- assumption that the given exp_stdev is calculated for a
        -- normal distribution.
        local exp_stdev_scaling = (1 / (#samples - 1)) ^ 0.5
        local new_scaling = (
            1 / ((#samples / MOMENT_SUBSAMPLES_BINS) - 1)
        ) ^ 0.5
        subsample_exp_stdev = exp_stdev * new_scaling / exp_stdev_scaling
        -- Note: Each subsample is likely to be closer to its subsample
        -- mean than to the overall mean... See:
        -- https://math.stackexchange.com/questions/1937346/shouldnt-sample-standard-deviation-decrease-with-increased-sample-size
        -- We don't account for that effect here...
    end

    -- Capture direction of deviation for each subsample
    local segment_deviations = {}
    local slice_deviations = {}

    -- Size of each subsample bin
    local bin_size = floor(#samples / MOMENT_SUBSAMPLES_BINS)

    for i = 1,MOMENT_SUBSAMPLES_BINS do
        -- Derive two subsamples: one based on segmenting the original
        -- sample and another based on collecting from indices with
        -- equal modulus.
        local sub_segment = {}
        local segment_mean = 0
        local sub_slice = {}
        local slice_mean = 0
        for j = 1,bin_size do
            local segment_next = samples[(i - 1) * bin_size + j]
            sub_segment[#sub_segment + 1] = segment_next
            segment_mean = segment_mean + segment_next

            local slice_next = samples[i + (j - 1) * MOMENT_SUBSAMPLES_BINS]
            sub_slice[#sub_slice + 1] = slice_next
            slice_mean = slice_mean + slice_next
        end
        segment_mean = segment_mean / #sub_segment
        slice_mean = slice_mean / #sub_slice
        if segment_mean < exp_mean then
            segment_deviations[#segment_deviations + 1] = true
        elseif segment_mean < exp_mean then
            segment_deviations[#segment_deviations + 1] = false
        end
        -- Don't add a deviation sample if the mean is miraculously
        -- exactly correct...
        if slice_mean < exp_mean then
            slice_deviations[#slice_deviations + 1] = true
        elseif slice_mean < exp_mean then
            slice_deviations[#slice_deviations + 1] = false
        end
        moments_test(
            sub_segment,
            exp_mean,
            subsample_exp_stdev,
            label .. " (subsample segment " .. i .. ")"
        )
        moments_test(
            sub_slice,
            exp_mean,
            subsample_exp_stdev,
            label .. " (subsample slice " .. i .. ")"
        )
    end
    binomial_samples_test(
        segment_deviations,
        0.5,
        20,
        label .. " (segments deviation directions)"
    )
    binomial_samples_test(
        slice_deviations,
        0.5,
        20,
        label .. " (slices deviation directions)"
    )
end


function trapezoid_area(height, top, bottom)
    -- Computes the area of a trapezoid with the given height and top/bottom
    -- lengths.
    return (
        -- triangle based on longer - shorter of the top/bottom
        0.5 * abs(top - bottom) * height
        -- plus parallelogram based on shorter edge
      + min(top, bottom) * height
    )
end


function cdf_points(low, high)
    -- Returns a list of N_CDF_BUCKETS + 1 evenly-distributed test points
    -- starting at low and ending at high.
    local result = {}
    for i = 1,N_CDF_BUCKETS do
        result[i] = low + ((i - 1) / N_CDF_BUCKETS) * (high - low)
    end
    result[#result + 1] = high
    return result
end


function cdf_test(
    samples,
    cdf,
    test_points,
    label,
    area_tolerance,
    cumulative_tolerance,
    bucket_tolerance
)
    -- Tests that the cumulative distribution function of the given
    -- samples roughly matches the given expected cumulative distribution
    -- function (should be a function which accepts a number x and
    -- returns a probability of the result being smaller than x). This
    -- estimates:
    -- 1. The total fractional difference between the number of samples
    --     in each bucket and the expected number based on the given CDF.
    -- 2. The total fractional difference between the cumulative sample
    --     count and expected cumulative sample count at each of the test
    --     points.
    -- 3. The total area of the differences between the actual and
    --     expected CDFs, sampling at each of the given test points.
    --
    -- Each of these must be within the respective tolerance level, with
    -- defaults based on the number of samples. By default, the tolerance
    -- for each difference is the result of the `tolerance` function,
    -- called based on the relevant number of samples.
    --
    -- This function uses asserts to test the outcome; the label is used
    -- for messaging.
    local ns = #samples
    if area_tolerance == nil then
        area_tolerance = tolerance(ns)
    end
    -- Let cumulative_tolerance remain nil to scale as we progress
    -- Set bucket_tolerance below when we know bucket sizes

    -- make a copy and sort it
    ordered = {}
    for i, v in ipairs(samples) do
        ordered[i] = v
    end
    table.sort(ordered)

    local discrepancy_area = 0
    local correct_area = 0
    local obs_toindex = 1

    -- Initialize previous variables for our next step
    local prev_exp_pc = 0
    local prev_precount = 0
    local prev_overshoot = 0
    local prev_point = 0

    for i, tp in ipairs(test_points) do
        exp_precount = cdf(tp) * ns
        while obs_toindex <= ns and ordered[obs_toindex] < tp do
            obs_toindex = obs_toindex + 1
        end

        local obs_precount = obs_toindex - 1

        -- Compute overshoot at this test point
        local overshoot = obs_precount - exp_precount

        local prev_bucket_exp = exp_precount - prev_exp_pc
        local prev_bucket_count = obs_precount - prev_precount

        -- Check that the cumulative count is within tolerance
        local cumulative_discrepancy = (
            (obs_precount - exp_precount)
          / exp_precount
        )
        local cumulative_tolerance_now = cumulative_tolerance
        if cumulative_tolerance_now == nil then
            -- Set it based on precount so far, so that it narrows as
            -- we've seen more of the CDF
            cumulative_tolerance_now = tolerance(exp_precount)
        end
        if exp_precount == 0 then
            cumulative_discrepancy = obs_precount  -- not a percentage
        end

        assert(
            cumulative_discrepancy < cumulative_tolerance_now,
            string.format(
                (
                    "Suspicious CDF cumulative count difference from %.2f for"
                 .. " %s values less than %.5f: %d -> %.2f%% > %.2f%%"
                ),
                exp_precount,
                label,
                tp,
                obs_precount,
                100 * cumulative_discrepancy,
                100 * cumulative_tolerance_now
            )
        )

        -- Check that the local bucket count is within tolerance
        local bucket_discrepancy = (
            (prev_bucket_count - prev_bucket_exp)
          / prev_bucket_exp
        )
        local this_bucket_tolerance = bucket_tolerance
        if this_bucket_tolerance == nil then
            this_bucket_tolerance = tolerance(prev_bucket_exp)
        end
        if prev_bucket_exp == 0 then
            bucket_discrepancy = prev_bucket_count  -- not a percentage...
        end
        assert(
            bucket_discrepancy < this_bucket_tolerance,
            string.format(
                (
                    "Suspicious CDF bucket count difference from %.2f for"
                 .. " %s bucket %.5f-%.5f: %d -> %.2f%% > %.2f%%"
                ),
                prev_bucket_exp,
                label,
                prev_point,
                tp,
                prev_bucket_count,
                100 * bucket_discrepancy,
                100 * this_bucket_tolerance
            )
        )

        -- Only for the 2nd+ test points...
        if i > 1 then
            -- compute top and bottom of (possibly twisted) trapezoid
            width = tp - test_points[i - 1]

            -- update correct area
            correct_area = (
                correct_area
              + trapezoid_area(width, prev_exp_pc, exp_precount)
            )

            -- update discrepancy area...
            if (prev_overshoot > 0) == (overshoot > 0) then
                -- it's a trapezoid; both sides either over- or
                -- under-shot.
                discrepancy_area = discrepancy_area + trapezoid_area(
                    width,
                    abs(prev_overshoot),
                    abs(overshoot)
                )
            else
                -- it's two triangles; one side did the opposite of the other.
                local inflection
                if overshoot ~= 0 then
                    local ratio = abs(prev_overshoot / overshoot)
                    inflection = width * ratio / (1 + ratio)
                else
                    inflection = width
                end
                discrepancy_area = discrepancy_area + (
                    -- triangle from prev test point to
                    -- inflection point
                    0.5 * abs(prev_overshoot) * inflection
                    -- triangle from inflection point to current
                    -- test point
                  + 0.5 * abs(overshoot) * (width - inflection)
                )
            end
        end

        -- Update previous variables for our next step
        prev_exp_pc = exp_precount
        prev_precount = obs_precount
        prev_overshoot = overshoot
        prev_point = tp
    end

    discrepancy = abs(discrepancy_area / correct_area)
    assert(discrepancy <= area_tolerance, (
        string.format(
            (
                "Suspicious CDF area discrepancy from (%.2f) for"
             .. " %s: %.2f -> %.2f%% > %.2f%%"
            ),
            correct_area,
            label,
            discrepancy_area,
            100 * discrepancy,
            100 * area_tolerance
        )
    ))
end


-- Report tolerance that we'll use
print(
    string.format(
        (
            "\nUsing %d samples, the default tolerance will be"
         .. " %0.2f%%.\nTolerances for CDF buckets would be:\n"
         .. "  %d -> %.2f%%\n  %d -> %.2f%%\n"
        ),
        N_SAMPLES,
        100 * tolerance(N_SAMPLES),
        N_SAMPLES / 10,
        100 * tolerance(N_SAMPLES / 10),
        N_SAMPLES / 100,
        100 * tolerance(N_SAMPLES / 100)
    )
)


function test_uniform_distribution()
    -- Tests the mean, standard deviation, and CDF for uniform results using
    -- N_SAMPLES samples starting from each of a few different seeds.
    start_progress("uniform::managed", #TEST_VALUES * N_SAMPLES)
    -- TODO: How to make the distribution better and/or test better?
    for _, seed in ipairs(TEST_VALUES) do
        local rand = seed
        local samples = {}
        for _ = 1,N_SAMPLES do
            samples[#samples + 1] = rng.uniform(rand)
            rand = rng.prng(rand, seed)
            progress()
        end

        moments_and_subsamples_test(
            samples,
            0.5,
            1 / 12 ^ 0.5,
            "uniform(:" .. seed .. ":)"
        )

        cdf_test(
            samples,
            function(x) return x end,
            cdf_points(0, 1),
            "uniform(:" .. seed .. ":)"
        )
    end

    samples = {}
    start_progress("uniform::sequential", #SEQ_SEEDS)
    for _, seed in ipairs(SEQ_SEEDS) do
        samples[#samples + 1] = rng.uniform(seed)
        progress()
    end

    moments_and_subsamples_test(
        samples,
        0.5,
        1 / 12 ^ 0.5,
        "uniform(:sequential:)"
    )

    cdf_test(
        samples,
        function(x) return x end,
        cdf_points(0, 1),
        "uniform(:sequential:)"
    )
end
test_uniform_distribution()


function test_normalish_distribution()
    -- Like `test_uniform_distribution`, but tests normalish. Does not test
    -- the CDF. (TODO: That?)
    start_progress("normalish::managed", #TEST_VALUES * N_SAMPLES)
    for _, seed in ipairs(TEST_VALUES) do
        local rand = seed
        local samples = {}
        for i = 1,N_SAMPLES do
            samples[#samples + 1] = rng.normalish(rand)
            rand = rng.prng(rand, seed)
            progress()
        end

        moments_and_subsamples_test(
            samples,
            0.5,
            1 / 6, -- see js/anarchy_tests.js for derivation
            "normalish(:" .. seed .. ":)"
        )
    end

    -- TODO: CDF test here?

    samples = {}
    start_progress("normalish::sequential", #SEQ_SEEDS)
    for _, seed in ipairs(SEQ_SEEDS) do
        samples[#samples + 1] = rng.normalish(seed)
        progress()
    end

    moments_and_subsamples_test(
        samples,
        0.5,
        1 / 6,
        "normalish(:sequential:)"
    )
end
test_normalish_distribution()


function test_flip_distribution()
    -- Tests the mean and standard deviation for the flip function using a few
    -- different probabilities and a few different seeds at each probability.
    local test_with = { 0.5, 0.2, 0.005, 0.9, 0.99 }
    start_progress(
        "flip::managed",
        (#test_with - 1) * #TEST_VALUES * N_SAMPLES
      + #TEST_VALUES * N_SAMPLES * 8
    )
    for _, p in ipairs(test_with) do
        for _, seed in ipairs(TEST_VALUES) do
            local rand = seed
            local samples = {}
            local n_samples = N_SAMPLES
            local tol = binomial_tolerance(n_samples)
            if p < 0.015 or (1 - p) < 0.015 then
                tol = tol * 3
                n_samples = n_samples * 6
                -- we increase both tolerance and number of samples
                -- (making it easier to hit that tolerance) for low and
                -- high probability trials
            end
            for i = 1,n_samples do
                samples[#samples + 1] = rng.flip(p, rand)
                rand = rng.prng(rand, seed)
                progress()
            end

            binomial_samples_test(
                samples,
                p,
                100, -- use 100 groups to calculate standard deviation
                "flip(" .. p .. ", :" .. seed .. ":)",
                tol
            )
        end
    end

    samples = {}
    start_progress("flip::sequential", #SEQ_SEEDS)
    for i, seed in ipairs(SEQ_SEEDS) do
        samples[#samples + 1] = rng.flip(0.5, seed)
        progress()
    end

    binomial_samples_test(
        samples,
        0.5,
        100,  -- 100 groups
        "flip(0.5, :sequential:)"
    )
end
test_flip_distribution()


function test_integer_distribution()
    -- Tests the mean, stdev, and CDF for `anarchy.rng.integer` using
    -- various low/high values, including inverted pairs.
    local lows = { 0, -31, 1289, -7294712 }
    local highs = { 0, -30, 1289482, -7298392 }
    start_progress(
        "integer::managed",
        #lows * #highs * #TEST_VALUES * N_SAMPLES
    )
    for _, low in ipairs(lows) do
        for _, high in ipairs(highs) do
            for _, seed in ipairs(TEST_VALUES) do
                local rand = seed
                local samples = {}
                for i = 1,N_SAMPLES do
                    samples[#samples + 1] = rng.integer(rand, low, high)
                    rand = rng.prng(rand, seed)
                    progress()
                end

                if low == high then
                    -- No need for mean/stdev/cdf tests here
                    for _, s in ipairs(samples) do
                        assert(
                            s == low,
                            (
                                "Non-fixed sample for integer(:"
                             .. seed .. ":, " .. low .. ", " .. high
                             .. "): " .. s
                            )
                        )
                    end
                else
                    local span = high - 1 - low
                    local exp_mean = low + span / 2
                    local exp_stdev = (1 / (12 ^ 0.5)) * abs(span)

                    moments_and_subsamples_test(
                        samples,
                        exp_mean,
                        exp_stdev,
                        (
                            "integer(:" .. seed .. ":, " .. low .. ", "
                         .. high .. ")"
                        )
                    )

                    local real_low = min(low, high)
                    local real_high = max(low, high)

                    local points
                    if high <= low + 1 then
                        points = { real_low, real_high + 1 }
                    else
                        points = cdf_points(real_low, real_high)
                    end
                    cdf_test(
                        samples,
                        function(x)
                            if real_high > real_low + 1 then
                                return min(
                                    1,
                                    ((x - real_low) / (real_high - real_low))
                                )
                            elseif x > real_low then
                                return 1
                            else
                                return 0
                            end
                            return 
                        end,
                        points,
                        (
                            "integer(:" .. seed .. ":, " .. low .. ", "
                         .. high .. ")"
                        )
                    )
                end
            end
        end
    end

    samples = {}
    low = -12
    high = 10472
    span = high - 1 - low
    exp_mean = low + span / 2
    exp_stdev = (1 / (12 ^ 0.5)) * abs(span)
    start_progress("integer::sequential", #SEQ_SEEDS)
    for _, seed in ipairs(SEQ_SEEDS) do
        samples[#samples + 1] = rng.integer(seed, low, high)
        progress()
    end

    moments_and_subsamples_test(
        samples,
        exp_mean,
        exp_stdev,
        "integer(:sequential:, " .. low .. ", " .. high .. ")"
    )

    cdf_test(
        samples,
        function(x) return min(1, ((x - low) / (high - low) )) end,
        cdf_points(low, high),
        "integer(:sequential:, " .. low .. ", " .. high .. ")"
    )
end
test_integer_distribution()


function test_exponential_distribution()
    -- Tests the distribution of exponential means, stdevs, and CDFs for
    -- several different seeds at each of several different shape values.
    local shapes = { 0.05, 0.5, 1, 1.5, 5 }
    start_progress(
        "exponential::managed",
        #shapes * #TEST_VALUES * N_SAMPLES
    )
    for _, shape in ipairs(shapes) do
        -- Test with different shape (lambda) values
        for _, seed in ipairs(TEST_VALUES) do
            local rand = seed
            local samples = {}

            local exp_mean = 1 / shape
            -- expected stdev is the same as the expected mean for an
            -- exponential distribution
            local exp_stdev = exp_mean

            -- compute samples
            for i = 1,N_SAMPLES do
                samples[#samples + 1] = rng.exponential(rand, shape)
                rand = rng.prng(rand, seed)
                progress()
            end

            -- TODO: A moments_and_subsamples_test here instead?
            moments_test(
                samples,
                exp_mean,
                exp_stdev,
                "exponential(:" .. seed .. ":, " .. shape .. ")"
            )

            local exp_cdf = cdf_points(0, 2)
            for _, p in ipairs(cdf_points(2.5, 30)) do
                exp_cdf[#exp_cdf + 1] = p
            end
            cdf_test(
                samples,
                function (x) return 1 - exp(-shape * x) end,
                exp_cdf,
                "exponential(:" .. seed .. ":, " .. shape .. ")"
            )
        end
    end

    samples = {}
    local shape = 0.75
    exp_mean = 1 / shape
    exp_stdev = exp_mean
    start_progress("exponential::sequential", #SEQ_SEEDS)
    for _, seed in ipairs(SEQ_SEEDS) do
        samples[#samples + 1] = rng.exponential(seed, shape)
        progress()
    end

    -- TODO: A moments_and_subsamples_test here instead?
    moments_test(
        samples,
        exp_mean,
        exp_stdev,
        "exponential(:sequential:, " .. shape .. ")"
    )

    cdf = cdf_points(0, 2)
    for _, p in ipairs(cdf_points(2.5, 30)) do
        cdf[#cdf + 1] = p
    end
    cdf_test(
        samples,
        function(x) return 1 - exp(-shape * x) end,
        cdf,
        "exponential(:sequential:, " .. shape .. ")"
    )
end
test_exponential_distribution()


function test_truncated_exponential_distribution()
    -- Tests `anarchy.rng.truncated_exponential` results at several
    -- different shape values. Tests means and CDFS but not stdevs (TODO:
    -- that?).
    local shapes = { 0.05, 0.5, 1, 1.5, 5 }
    start_progress(
        "truncated_exponential::managed",
        #shapes * #TEST_VALUES * N_SAMPLES
    )
    for _, shape in ipairs(shapes) do
        -- Test with different shape (lambda) values
        for _, seed in ipairs(TEST_VALUES) do
            local rand = seed
            local samples = {}

            local exp_mean = (1 / shape) - (1 / (exp(shape) - 1))
            -- TODO: What's the expected stdev here?
            local exp_stdev = nil

            -- compute samples
            for i = 1,N_SAMPLES do
                samples[#samples + 1] = rng.truncated_exponential(rand, shape)
                rand = rng.prng(rand, seed)
                progress()
            end

            -- TODO: A moments_and_subsamples_test here instead?
            moments_test(
                samples,
                exp_mean,
                exp_stdev,
                "truncated_exponential(:" .. seed .. ":, " .. shape .. ")"
            )

            -- Note: js/anarchy_tests.js contains a derivation of the
            -- truncated CDF formula we're using here
            cdf_test(
                samples,
                function(x)
                    return (1 - exp(-shape * x)) / (1 - exp(-shape))
                end,
                cdf_points(0, 1),
                "truncated_exponential(:" .. seed .. ":, " .. shape .. ")"
            )
        end
    end

    samples = {}
    shape = 0.75
    exp_mean = (1 / shape) - (1 / (exp(shape) - 1))
    exp_stdev = nil  -- TODO
    start_progress("truncated_exponential::sequential", #SEQ_SEEDS)
    for _, seed in ipairs(SEQ_SEEDS) do
        samples[#samples + 1] = rng.truncated_exponential(seed, shape)
    end

    -- TODO: A moments_and_subsamples_test here instead?
    moments_test(
        samples,
        exp_mean,
        exp_stdev,
        "truncated_exponential(:sequential:, " .. shape .. ")"
    )

    cdf_test(
        samples,
        function(x)
            return (1 - exp(-shape * x)) / (1 - exp(-shape))
        end,
        cdf_points(0, 1),
        "truncated_exponential(:sequential:, " .. shape .. ")"
    )
end
test_truncated_exponential_distribution()


--=======--
-- Tests --
--=======--

-- Straightforward value tests...

function test_posmod()
    assert(util.posmod(-1, 7) == 6)
    assert(util.posmod(0, 11) == 0)
    assert(util.posmod(3, 11) == 3)
    assert(util.posmod(-13, 11) == 9)
    assert(util.posmod(15, 11) == 4)
    assert(util.posmod(115, 10) == 5)
    assert(util.posmod(115, 100) == 15)
end
test_posmod()


function test_mask()
    assert(util.mask(0) == 0)
    assert(util.mask(1) == 1)
    assert(util.mask(2) == 3)
    assert(util.mask(4) == 15)
    assert(util.mask(10) == 1023)
end
test_mask()


function test_byte_mask()
    assert(util.byte_mask(0) == 255)
    assert(util.byte_mask(1) == 65280)
    assert(util.byte_mask(2) == 16711680)
end
test_byte_mask()


function test_unit_ops()
    local seed = 129873918231
    start_progress("ops", 5 * bit.lshift(1, 12) * #TEST_VALUES)
    for _, o in ipairs{0, 12, 24, 36, 48} do
        for i = 0,bit.lshift(1, 12) do
            local x = bit.lshift(i, o)
            assert(x == rng.flop(rng.flop(x)))
            assert(x == rng.rev_scramble(rng.scramble(x)))
            assert(x == rng.prng(rng.rev_prng(x, seed), seed))
            local r = x
            local p = x
            for j, y in ipairs(TEST_VALUES) do
                assert(x == rng.rev_swirl(rng.swirl(x, y), y))
                assert(x == rng.fold(rng.fold(x, y), y))
                assert(r == rng.scramble(rng.rev_scramble(r)))
                local r = rng.rev_scramble(r)
                assert(p == rng.rev_prng(rng.prng(p, seed), seed))
                local p = rng.prng(p, seed)
                progress()
            end
        end
    end
end


function test_swirl()
    assert(rng.swirl(2, 1) == 1)
    assert(rng.swirl(4, 1) == 2)
    assert(rng.swirl(8, 2) == 2)
    assert(rng.swirl(1, 1) == 0x8000000000000000)
    assert(rng.swirl(2, 2) == 0x8000000000000000)
    assert(rng.swirl(1, 2) == 0x4000000000000000)
    assert(rng.rev_swirl(1, 2) == 4)
    assert(rng.rev_swirl(1, 3) == 8)
    assert(rng.rev_swirl(2, 2) == 8)
    assert(rng.rev_swirl(0x8000000000000000, 1) == 1)
    assert(rng.rev_swirl(0x8000000000000000, 2) == 2)
    assert(rng.rev_swirl(0x8000000000000000, 3) == 4)
    assert(rng.rev_swirl(0x4000000000000000, 3) == 2)
    assert(rng.rev_swirl(0x0010000000001030, 1) == 0x0020000000002060)
    assert(rng.rev_swirl(rng.swirl(1098301, 17), 17) == 1098301)
end
test_swirl()


function test_fold()
    local f
    -- Test that it's its own inverse
    f = rng.fold(rng.fold(18201, 18), 18)
    assert(f == 18201, f)
    f = rng.fold(rng.fold(89348393, 1289), 1289)
    assert(f == 89348393, f)

    if anarchy.ID_BYTES == 4 then
        -- Should fold 10 bits (8-16 range w/ 2 index)
        -- 0x747 (minus initial 01) ->
        -- 11 01 00 01 11
        -- Folding onto upper 10 bits, which are:
        -- 00 01 00 11 00
        -- So we get:
        -- 11 00 00 10 11
        -- Which is (along with extra 0x01): 0xc2d
        f = rng.fold(0x13134747, 2)
        assert(f == 0xc2d34747, f)

        -- Random values checked & locked in
        f = rng.fold(22908, 7)
        assert(f == 3002620284, f)
        f = rng.fold(18201, 18)
        assert(f == 3326101273, f)
    else
        -- Should fold 18 bits (16-32 range w/ 2 index)
        -- 0x34747 (minus two initial zeroes) ->
        -- 11 01 00 01 11 01 00 01 11
        -- Folding onto upper 18 bits, which are:
        -- 00 01 00 11 00 01 00 11 00
        -- So we get:
        -- 11 00 00 10 11 00 00 10 11
        -- Which is (along with extra 0x01): 0xc2c2d
        f = rng.fold(0x1313131313134747, 2)
        assert(f == 0xc2c2d31313134747, f)

        -- Random values checked & locked in
        f = rng.fold(22908, 7)
        assert(f == 50375224738208124, f)
        f = rng.fold(18201, 18)
        assert(f == 1280781512777680665, f)

        -- 8-byte inverse tests
        f = rng.fold(rng.fold(0x0000f37d1ac247eb, 23982), 23982)
        assert(f == 0x0000f37d1ac247eb, f)
        f = rng.fold(rng.fold(0xfff4422ffff4422, 0), 0)
        assert(f == 0xfff4422ffff4422, f)
    end
end
test_fold()


function test_flop()
    assert(rng.flop(0xf0f0f0f0) == 0x0f0f0f0f)
    assert(rng.flop(0x0a0a0a0a) == 0xa0a0a0a0)
    assert(rng.flop(22908) == 38343)
    assert(rng.flop(18201) == 29841)
    assert(rng.flop(rng.flop(3892389)) == 3892389)
    assert(rng.flop(rng.flop(489248448)) == 489248448)
    if anarchy.ID_BYTES == 8 then
        assert(rng.flop(0x7070707070707070) == 0x0707070707070707)
        assert(rng.flop(0x0e0e0e0e0e0e0e0e) == 0xe0e0e0e0e0e0e0e0)
    end
end
test_flop()


function test_scramble()
    local t, s
    -- Test value that has no overlap with the scramble set bits, but
    -- which will trigger scrambling even when shifted left by 1
    t = 0x40004001
    assert(
        (t & rng.SCRAMBLE_SET) == 0,
        string.format(
            "%x overlaps with %x: %x",
            t,
            rng.SCRAMBLE_SET,
            (t & rng.SCRAMBLE_SET)
        )
    )
    -- OR-in scramble set bits, shift left, and scramble
    -- Scramble bits should knock themselves out leaving the t value
    -- alone
    s = rng.scramble(rng.rev_swirl(rng.SCRAMBLE_SET | t, 1))
    assert(s == 0x40004001, s)
    s = rng.rev_scramble(0x40004001)
    assert(s == rng.rev_swirl(rng.SCRAMBLE_SET | 0x40004001, 1), s)
    s = rng.rev_scramble(rng.scramble(17))
    assert(s == 17, s)
    s = rng.rev_scramble(rng.scramble(8493489))
    assert(s == 8493489, s)
    s = rng.scramble(rng.rev_scramble(8493489))
    assert(s == 8493489, s)
    if anarchy.ID_BYTES == 8 then
        local s = rng.scramble(rng.rev_scramble(0x378294ef7ab3301))
        assert(s == 0x378294ef7ab3301, s)
    end
    s = rng.scramble(rng.rev_scramble(0))
    assert(s == 0, s)
    s = rng.rev_scramble(rng.scramble(0))
    assert(s == 0, s)
end
test_scramble()


function test_prng()
    local seed, start, expect, result
    local test_values
    assert(rng.prng(rng.rev_prng(1782, 39823), 39823) == 1782)
    assert(rng.rev_prng(rng.prng(1782, 39823), 39823) == 1782)
    test_values = {
        {373891, 489348, 1541976216},
        {0, 0, -1409750063},
        {28983, 389, 0},
    }
    if anarchy.ID_BYTES == 8 then
        seed = 4938433849834
        start = 98349833984983
        result = rng.prng(rng.rev_prng(start, seed), seed) 
        assert(result == start, "" .. result .. " </> " .. start)
        seed = 0x803451657483efa7
        start = 0x0493f3a8dec4d4aa
        result = rng.prng(rng.rev_prng(start, seed), seed) 
        assert(result == start, "" .. result .. " </> " .. start)
        assert(rng.rev_prng(rng.prng(0, 483984), 483984) == 0)
        assert(rng.prng(rng.rev_prng(0, 483984), 483984) == 0)
        test_values = {
            {373891, 489348, 7811360407439714725},
            {0, 0, 131877779694904891},
            {28983, 389, -1691726202175399180},
            {483984, 2489308509186683412, 0},
        }
    end

    for _, tvs in ipairs(test_values) do
        seed = tvs[1]
        start = tvs[2]
        expect = tvs[3]
        result = rng.prng(start, seed)
        assert(
            result == expect,
            string.format("%d: %d ? %d", seed, result, expect)
        )
        assert(rng.rev_prng(result, seed) == start)
    end

    local n = 298398
    for s = 34878374,34878374 + 100 do
        local nx = rng.prng(n, s)
        assert(rng.rev_prng(nx, s) == n)
    end

    for s = 1,30 do
        local sd = 18373 + 3289832*s
        for i = 1,100 do
            nx = rng.prng(n, sd)
            assert(rng.rev_prng(nx, sd) == n)
            n = nx  -- continue
        end
    end
end
test_prng()


function test_lfsr()
    -- TODO: Verify these!
    assert(rng.lfsr(489348) == 244674)
    assert(rng.lfsr(1766932808) == 883466404)
end
test_lfsr()


function test_uniform()
    if anarchy.ID_BYTES == 4 then
        assert(
            rng.uniform(0)
         ==  0.16982559633064941984059714741306379437446594238281250,
            string.format("%.53f", rng.uniform(0))
        )
        assert(
            rng.uniform(8329801)
         ==  0.57223131074824162833891705304267816245555877685546875,
            string.format("%.53f", rng.uniform(8329801))
        )
        assert(
            rng.uniform(58923)
         ==  0.26524206860099791560614335139689501374959945678710938,
            string.format("%.53f", rng.uniform(58923))
        )
    elseif anarchy.ID_BYTES == 8 then
        assert(
            rng.uniform(0)
         ==   0.41885797171245592318555850397387985140085220336914062,
            string.format("%.53f", rng.uniform(0))
        )
        assert(
            rng.uniform(8329801)
         == 0.99183482023694691243065335584105923771858215332031250,
            string.format("%.53f", rng.uniform(8329801))
        )
        assert(
            rng.uniform(58923)
         == 0.68184228287304127213985793787287548184394836425781250,
            string.format("%.53f", rng.uniform(58923))
        )
    else
        assert(false, "Unexpected ID bytes: " .. anarchy.ID_BYTES)
    end
end
test_uniform()


function test_normalish()
    if anarchy.ID_BYTES == 4 then
        assert(
            rng.normalish(0)
         ==  0.48090422481456157610679724712099414318799972534179688,
            string.format("%.53f", rng.normalish(0))
        )
        assert(
            rng.normalish(8329801)
         == 0.99183482023694691243065335584105923771858215332031250,
            string.format("%.53f", rng.normalish(8329801))
        )
        assert(
            rng.normalish(58923)
         == 0.46549958295476212555286110728047788143157958984375000,
            string.format("%.53f", rng.normalish(58923))
        )
    elseif anarchy.ID_BYTES == 8 then
        assert(
            rng.normalish(0)
         == 0.62947778336024839962448140795459039509296417236328125,
            string.format("%.53f", rng.normalish(0))
        )
        assert(
            rng.normalish(8329801)
         == 0.28449636259630423085198458466038573533296585083007812,
            string.format("%.53f", rng.normalish(8329801))
        )
        assert(
            rng.normalish(58923)
         == 0.56064914785989483636541308442247100174427032470703125,
            string.format("%.53f", rng.normalish(58923))
        )
    else
        assert(false, "Unexpected ID bytes: " .. anarchy.ID_BYTES)
    end
end
test_normalish()


function test_integer()
    local r
    if anarchy.ID_BYTES == 4 then
        r = rng.integer(0, 0, 1)
        assert(r == 0, r)
        r = rng.integer(0, 3, 25)
        assert(r == 6, r)
        r = rng.integer(1, 3, 25)
        assert(r == 20, r)
        r = rng.integer(2, 3, 25)
        assert(r == 17, r)
        r = rng.integer(58923, 3, 25)
        assert(r == 8, r)
        r = rng.integer(58923, -2, -4)
        assert(r == -3, r)
        r = rng.integer(58923, -20, -40)
        assert(r == -26, r)
    elseif anarchy.ID_BYTES == 8 then
        r = rng.integer(0, 0, 1)
        assert(r == 0, r)
        r = rng.integer(0, 3, 25)
        assert(r == 12, r)
        r = rng.integer(1, 3, 25)
        assert(r == 11, r)
        r = rng.integer(2, 3, 25)
        assert(r == 23, r)
        r = rng.integer(58923, 3, 25)
        assert(r == 18, r)
        r = rng.integer(58923, -2, -4)
        assert(r == -4, r)
        r = rng.integer(58923, -20, -40)
        assert(r == -34, r)
    else
        assert(false, "Unexpected ID bytes: " .. anarchy.ID_BYTES)
    end
end
test_integer()


function test_exponential()
    if anarchy.ID_BYTES == 4 then
        assert(
            rng.exponential(0, 0.5)
         == 3.54596654493773444372095582366455346345901489257812500,
            string.format("%.53f", rng.exponential(0, 0.5))
        )
        assert(
            rng.exponential(8329801, 0.5)
         == 1.11642395985140230330046051676617935299873352050781250,
            string.format("%.53f", rng.exponential(8329801, 0.5))
        )
        assert(
            rng.exponential(58923, 1.5)
         == 0.88474160235573739985426300336257554590702056884765625,
            string.format("%.53f", rng.exponential(58923, 1.5))
        )
    elseif anarchy.ID_BYTES == 8 then
    else
        assert(false, "Unexpected ID bytes: " .. anarchy.ID_BYTES)
    end
end
test_exponential()


function test_truncated_exponential()
    if anarchy.ID_BYTES == 4 then
        assert(
            rng.truncated_exponential(0, 0.5)
         == 0.54596654493773444372095582366455346345901489257812500,
            string.format("%.53f", rng.truncated_exponential(0, 0.5))
        )
        assert(
            rng.truncated_exponential(8329801, 1.5)
         == 0.37214131995046745293720391600800212472677230834960938,
            string.format("%.53f", rng.truncated_exponential(8329801, 1.5))
        )
        assert(
            rng.truncated_exponential(58923, 2.5)
         == 0.53084496141344250652593927952693775296211242675781250,
            string.format("%.53f", rng.truncated_exponential(58923, 2.5))
        )
    elseif anarchy.ID_BYTES == 8 then
        assert(
            rng.truncated_exponential(0, 0.5)
         == 0.74044677236642031381563811009982600808143615722656250,
            string.format("%.53f", rng.truncated_exponential(0, 0.5))
        )
        assert(
            rng.truncated_exponential(8329801, 1.5)
         == 0.00546579827987946423317744404357654275372624397277832,
            string.format("%.53f", rng.truncated_exponential(8329801, 1.5))
        )
        assert(
            rng.truncated_exponential(58923, 2.5)
         == 0.15318276186681811790180063326261006295680999755859375,
            string.format("%.53f", rng.truncated_exponential(58923, 2.5))
        )
    end
end
test_truncated_exponential()


function test_cohorts()
    assert(
        anarchy.cohort.cohort(17, 3) == 6,
        "" .. anarchy.cohort.cohort(17, 3)
    )
    assert(
        anarchy.cohort.cohort(-1, 10) == 0,
        "" .. anarchy.cohort.cohort(-1, 10)
    )
    assert(
        anarchy.cohort.cohort(0, 10) == 0,
        "" .. anarchy.cohort.cohort(0, 10)
    )
    assert(
        anarchy.cohort.cohort(10, 10) == 1,
        "" .. anarchy.cohort.cohort(10, 10)
    )
    assert(
        anarchy.cohort.cohort(9, 10) == 1,
        "" .. anarchy.cohort.cohort(9, 10)
    )
    assert(
        anarchy.cohort.cohort(11, 10) == 2,
        "" .. anarchy.cohort.cohort(11, 10)
    )
    assert(
        anarchy.cohort.cohort_inner(17, 3) == 2,
        "" .. anarchy.cohort.cohort_inner(17, 3)
    )
    assert(
        anarchy.cohort.cohort_inner(10, 10) == 10,
        "" .. anarchy.cohort.cohort_inner(10, 10)
    )
    assert(
        anarchy.cohort.cohort_inner(9, 10) == 9,
        "" .. anarchy.cohort.cohort_inner(9, 10)
    )
    c, i = anarchy.cohort.cohort_and_inner(9, 10)
    assert(c == 1, "" .. c)
    assert(i == 9, "" .. i)
    assert(
        anarchy.cohort.cohort_outer(1, 3, 112) == 3,
        anarchy.cohort.cohort_outer(1, 3, 112)
    )
    assert(
        anarchy.cohort.cohort_outer(2, 3, 112) == 115,
        anarchy.cohort.cohort_outer(2, 3, 112)
    )
    assert(
        anarchy.cohort.cohort_outer(0, 3, 112) == -109,
        anarchy.cohort.cohort_outer(0, 3, 112)
    )
end
test_cohorts()


function test_cohort_interleave()
    assert(anarchy.cohort.cohort_interleave(1, 3) == 2)
    assert(anarchy.cohort.cohort_interleave(2, 3) == 3)
    assert(anarchy.cohort.cohort_interleave(3, 3) == 1)
    assert(anarchy.cohort.rev_cohort_interleave(1, 3) == 3)
    assert(anarchy.cohort.rev_cohort_interleave(2, 3) == 1)
    assert(anarchy.cohort.rev_cohort_interleave(3, 3) == 2)

    assert(anarchy.cohort.cohort_interleave(1, 4) == 2)
    assert(anarchy.cohort.cohort_interleave(2, 4) == 4)
    assert(anarchy.cohort.cohort_interleave(3, 4) == 3)
    assert(anarchy.cohort.cohort_interleave(4, 4) == 1)
    assert(anarchy.cohort.rev_cohort_interleave(1, 4) == 4)
    assert(anarchy.cohort.rev_cohort_interleave(2, 4) == 1)
    assert(anarchy.cohort.rev_cohort_interleave(3, 4) == 3)
    assert(anarchy.cohort.rev_cohort_interleave(4, 4) == 2)

    assert(anarchy.cohort.cohort_interleave(1, 12) == 2)
    assert(anarchy.cohort.cohort_interleave(2, 12) == 4)
    assert(anarchy.cohort.cohort_interleave(3, 12) == 6)
    assert(anarchy.cohort.cohort_interleave(4, 12) == 8)
    assert(anarchy.cohort.cohort_interleave(5, 12) == 10)
    assert(anarchy.cohort.cohort_interleave(6, 12) == 12)
    assert(anarchy.cohort.cohort_interleave(7, 12) == 11)
    assert(anarchy.cohort.cohort_interleave(8, 12) == 9)
    assert(anarchy.cohort.cohort_interleave(9, 12) == 7)
    assert(anarchy.cohort.cohort_interleave(10, 12) == 5)
    assert(anarchy.cohort.cohort_interleave(11, 12) == 3)
    assert(anarchy.cohort.cohort_interleave(12, 12) == 1)

    assert(anarchy.cohort.rev_cohort_interleave(2, 12) == 1)
    assert(anarchy.cohort.rev_cohort_interleave(4, 12) == 2)
    assert(anarchy.cohort.rev_cohort_interleave(6, 12) == 3)
    assert(anarchy.cohort.rev_cohort_interleave(8, 12) == 4)
    assert(anarchy.cohort.rev_cohort_interleave(10, 12) == 5)
    assert(anarchy.cohort.rev_cohort_interleave(12, 12) == 6)
    assert(anarchy.cohort.rev_cohort_interleave(11, 12) == 7)
    assert(anarchy.cohort.rev_cohort_interleave(9, 12) == 8)
    assert(anarchy.cohort.rev_cohort_interleave(7, 12) == 9)
    assert(anarchy.cohort.rev_cohort_interleave(5, 12) == 10)
    assert(anarchy.cohort.rev_cohort_interleave(3, 12) == 11)
    assert(anarchy.cohort.rev_cohort_interleave(1, 12) == 12)
end
test_cohort_interleave()



function test_cohort_fold()
    assert(
        anarchy.cohort.cohort_fold(1, 3, 0) == 1,
        anarchy.cohort.cohort_fold(1, 3, 0)
    )
    assert(
        anarchy.cohort.cohort_fold(2, 3, 0) == 3,
        anarchy.cohort.cohort_fold(2, 3, 0)
    )
    assert(
        anarchy.cohort.cohort_fold(3, 3, 0) == 2,
        anarchy.cohort.cohort_fold(3, 3, 0)
    )
    assert(
        anarchy.cohort.rev_cohort_fold(1, 3, 0) == 1,
        anarchy.cohort.rev_cohort_fold(1, 3, 0)
    )
    assert(
        anarchy.cohort.rev_cohort_fold(2, 3, 0) == 3,
        anarchy.cohort.rev_cohort_fold(2, 3, 0)
    )
    assert(
        anarchy.cohort.rev_cohort_fold(3, 3, 0) == 2,
        anarchy.cohort.rev_cohort_fold(3, 3, 0)
    )
end
test_cohort_fold()


function test_cohort_spin()
    assert(anarchy.cohort.cohort_spin(1, 2, 1048239) == 2)
    assert(anarchy.cohort.rev_cohort_spin(2, 2, 1048239) == 1)
    assert(anarchy.cohort.cohort_spin(1, 10, 3) == 4)
    assert(anarchy.cohort.cohort_spin(2, 10, 3) == 5)
    assert(anarchy.cohort.rev_cohort_spin(5, 10, 3) == 2)
    assert(anarchy.cohort.cohort_spin(7, 10, 3) == 10)
    assert(anarchy.cohort.rev_cohort_spin(10, 10, 3) == 7)
    assert(anarchy.cohort.cohort_spin(8, 10, 3) == 1)
    assert(anarchy.cohort.rev_cohort_spin(1, 10, 3) == 8)
    assert(anarchy.cohort.cohort_spin(9, 10, 3) == 2)
    assert(anarchy.cohort.rev_cohort_spin(2, 10, 3) == 9)
end
test_cohort_spin()


function test_cohort_flop()
    assert(cohort.cohort_flop(1, 32, 3) == 6)  -- size will be 5
    assert(cohort.cohort_flop(2, 32, 3) == 7)
    assert(cohort.cohort_flop(3, 32, 3) == 8)
    assert(cohort.cohort_flop(4, 32, 3) == 9)
    assert(cohort.cohort_flop(5, 32, 3) == 10)
    assert(cohort.cohort_flop(6, 32, 3) == 1)
    assert(cohort.cohort_flop(7, 32, 3) == 2)
    assert(cohort.cohort_flop(8, 32, 3) == 3)
    assert(cohort.cohort_flop(9, 32, 3) == 4)
    assert(cohort.cohort_flop(10, 32, 3) == 5)
    assert(cohort.cohort_flop(21, 32, 3) == 26)
    assert(cohort.cohort_flop(30, 32, 3) == 25)
    assert(cohort.cohort_flop(31, 32, 3) == 31)
    assert(cohort.cohort_flop(32, 32, 3) == 32)
    assert(cohort.cohort_flop(1, 1024, 17) == 20)
    assert(cohort.cohort_flop(20, 1024, 17) == 1)
end
test_cohort_flop()


function test_cohort_mix()
    assert(
        anarchy.cohort.cohort_mix(1, 3, 0) == 3,
        anarchy.cohort.cohort_mix(1, 3, 0)
    )
    assert(
        anarchy.cohort.cohort_mix(2, 3, 0) == 2,
        anarchy.cohort.cohort_mix(2, 3, 0)
    )
    assert(
        anarchy.cohort.cohort_mix(3, 3, 0) == 1,
        anarchy.cohort.cohort_mix(2, 3, 0)
    )
    assert(
        anarchy.cohort.rev_cohort_mix(2, 3, 0) == 2,
        anarchy.cohort.rev_cohort_mix(2, 3, 0)
    )
    assert(
        anarchy.cohort.rev_cohort_mix(1, 3, 0) == 3,
        anarchy.cohort.rev_cohort_mix(1, 3, 0)
    )
    assert(
        anarchy.cohort.rev_cohort_mix(3, 3, 0) == 1,
        anarchy.cohort.rev_cohort_mix(3, 3, 0)
    )
end
test_cohort_mix()


function test_cohort_spread()
    -- Min regions 2, max regions 50, so 2 + 5 = 7 regions
    -- Region size 100 // 7 = 14
    -- 2 leftovers
    assert(cohort.cohort_spread(1, 100, 5) == 3)
    assert(cohort.cohort_spread(2, 100, 5) == 17)
    assert(cohort.cohort_spread(3, 100, 5) == 31)
    assert(cohort.cohort_spread(4, 100, 5) == 45)
    assert(cohort.cohort_spread(5, 100, 5) == 59)
    assert(cohort.cohort_spread(6, 100, 5) == 73)
    assert(cohort.cohort_spread(7, 100, 5) == 87)
    assert(cohort.cohort_spread(8, 100, 5) == 4)
    assert(cohort.cohort_spread(9, 100, 5) == 18)
    assert(cohort.cohort_spread(10, 100, 5) == 32)
    assert(cohort.cohort_spread(11, 100, 5) == 46)
    -- etc.
    assert(cohort.cohort_spread(15, 100, 5) == 5)
    assert(cohort.cohort_spread(16, 100, 5) == 19)
    -- Leftovers
    assert(cohort.cohort_spread(99, 100, 5) == 1)
    assert(cohort.cohort_spread(100, 100, 5) == 2)

    -- Same in reverse
    assert(cohort.rev_cohort_spread(3, 100, 5) == 1)
    assert(cohort.rev_cohort_spread(17, 100, 5) == 2)
    assert(cohort.rev_cohort_spread(31, 100, 5) == 3)
    assert(cohort.rev_cohort_spread(45, 100, 5) == 4)
    assert(cohort.rev_cohort_spread(59, 100, 5) == 5)
    assert(cohort.rev_cohort_spread(73, 100, 5) == 6)
    assert(cohort.rev_cohort_spread(87, 100, 5) == 7)
    assert(cohort.rev_cohort_spread(4, 100, 5) == 8)
    assert(cohort.rev_cohort_spread(18, 100, 5) == 9)
    assert(cohort.rev_cohort_spread(32, 100, 5) == 10)
    assert(cohort.rev_cohort_spread(46, 100, 5) == 11)
    -- etc.
    assert(cohort.rev_cohort_spread(5, 100, 5) == 15)
    assert(cohort.rev_cohort_spread(19, 100, 5) == 16)
    -- Leftovers
    assert(cohort.rev_cohort_spread(1, 100, 5) == 99)
    assert(cohort.rev_cohort_spread(2, 100, 5) == 100)

    -- Failed case from shuffle
    -- assert(cohort.rev_cohort_spread(3, 1024, 3) == 100)
end
test_cohort_spread()


function test_cohort_upend()
    -- 1 regions override
    assert(cohort.cohort_upend(1, 3, 0) == 3)
    assert(cohort.cohort_upend(2, 3, 0) == 2)
    assert(cohort.cohort_upend(3, 3, 0) == 1)
    -- 2 size-2 regions; no leftovers
    assert(cohort.cohort_upend(1, 4, 0) == 2)
    assert(cohort.cohort_upend(2, 4, 0) == 1)
    assert(cohort.cohort_upend(3, 4, 0) == 4)
    assert(cohort.cohort_upend(4, 4, 0) == 3)
    -- Min regions 2, max regions 50, so (2 + 25 % 16) = 11 regions
    -- Region size 100 // 11 = 9
    -- 1 leftover
    assert(cohort.cohort_upend(1, 100, 25) == 9)
    assert(cohort.cohort_upend(2, 100, 25) == 8)
    assert(cohort.cohort_upend(3, 100, 25) == 7)
    assert(cohort.cohort_upend(4, 100, 25) == 6)
    assert(cohort.cohort_upend(5, 100, 25) == 5)
    assert(cohort.cohort_upend(6, 100, 25) == 4)
    assert(cohort.cohort_upend(7, 100, 25) == 3)
    assert(cohort.cohort_upend(8, 100, 25) == 2)
    assert(cohort.cohort_upend(9, 100, 25) == 1)
    assert(cohort.cohort_upend(10, 100, 25) == 18)
    assert(cohort.cohort_upend(11, 100, 25) == 17)
    assert(cohort.cohort_upend(12, 100, 25) == 16)
    assert(cohort.cohort_upend(13, 100, 25) == 15)
    assert(cohort.cohort_upend(14, 100, 25) == 14)
    assert(cohort.cohort_upend(18, 100, 25) == 10)
    -- etc.
    -- here's the end of the last region + leftovers
    assert(cohort.cohort_upend(91, 100, 25) == 99)
    assert(cohort.cohort_upend(92, 100, 25) == 98)
    assert(cohort.cohort_upend(98, 100, 25) == 92)
    assert(cohort.cohort_upend(99, 100, 25) == 91)
    assert(cohort.cohort_upend(100, 100, 25) == 100)
end


function test_cohort_shuffle()
    -- size-3 cohorts w/ seed 17, forward/reverse
    local s1 = anarchy.cohort.cohort_shuffle(1, 3, 17)
    local s2 = anarchy.cohort.cohort_shuffle(2, 3, 17)
    local s3 = anarchy.cohort.cohort_shuffle(3, 3, 17)
    assert(s1 == 3, "" .. s1)
    assert(s2 == 1, "" .. s2)
    assert(s3 == 2, "" .. s3)
    local rs1 = anarchy.cohort.rev_cohort_shuffle(1, 3, 17)
    local rs2 = anarchy.cohort.rev_cohort_shuffle(2, 3, 17)
    local rs3 = anarchy.cohort.rev_cohort_shuffle(3, 3, 17)
    assert(rs1 == 2, "" .. rs1)
    assert(rs2 == 3, "" .. rs2)
    assert(rs3 == 1, "" .. rs3)

    -- size-1024 cohorts w/ seed 3, forward/reverse
    local exp = { 487, 468, 146, 445, 297, 20, 375, 361, 70, 889, 37, 427 }
    for i, e in ipairs(exp) do
        local shuf = anarchy.cohort.cohort_shuffle(i, 1024, 3)
        assert(shuf == e, "" .. i .. "→" .. shuf .. " ~= " .. e)
        local rshuf = anarchy.cohort.rev_cohort_shuffle(e, 1024, 3)
        assert(rshuf == i, "" .. e .. "←" .. rshuf .. " ~= " .. i)
    end

    -- Different seed
    local exp = { 225, 525, 595, 932, 843, 756, 862, 967, 269, 304, 283, 421 }
    for i, e in ipairs(exp) do
        local shuf = anarchy.cohort.cohort_shuffle(i, 1024, 2389832)
        assert(shuf == e, "" .. i .. "→" .. shuf .. " ~= " .. e)
        local rshuf = anarchy.cohort.rev_cohort_shuffle(e, 1024, 2389832)
        assert(rshuf == i, "" .. e .. "←" .. rshuf .. " ~= " .. i)
    end
end
test_cohort_shuffle()


function test_cohort_ops()
    -- Tests the cohort operations.
    local cohort_sizes = { 3, 12, 17, 32, 1024 }
    local cumulative_size = 0
    for _, s in ipairs(cohort_sizes) do
        cumulative_size = cumulative_size + s
    end
    start_progress(
        "cohort_ops",
        cumulative_size * (#TEST_VALUES + 1)
    )
    for idx = 1,#cohort_sizes do
        local cs = cohort_sizes[idx]
        observed = {
            interleave = {},
            fold = {},
            spin = {},
            flop = {},
            mix = {},
            spread = {},
            upend = {},
            shuffle = {},
        }
        for i = 1,cs do
            -- interleave
            local x = cohort.cohort_interleave(i, cs)
            local rc = cohort.rev_cohort_interleave(x, cs)
            assert(
                i == rc,
                "interleave(" .. i .. ", " .. cs .. ")→" .. x .. "→"
             .. rc .. ""
            )

            local v = tostring(x)
            if observed["interleave"][v] ~= nil then
                observed["interleave"][v] = observed["interleave"][v] + 1
            else
                observed["interleave"][v] = 1
            end
            progress()
        end

        for j, seed in ipairs(TEST_VALUES) do
            local js = tostring(j)
            observed["fold"][js] = {}
            observed["spin"][js] = {}
            observed["flop"][js] = {}
            observed["mix"][js] = {}
            observed["spread"][js] = {}
            observed["upend"][js] = {}
            observed["shuffle"][js] = {}
            for i = 1,cs do
                -- fold
                x = cohort.cohort_fold(i, cs, seed)
                rc = cohort.rev_cohort_fold(x, cs, seed)
                assert(
                    i == rc,
                    "fold(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["fold"][js][v] ~= nil then
                    observed["fold"][js][v] = observed["fold"][js][v] + 1
                else
                    observed["fold"][js][v] = 1
                end

                -- spin
                x = cohort.cohort_spin(i, cs, seed)
                rc = cohort.rev_cohort_spin(x, cs, seed)
                assert(
                    i == rc,
                    "spin(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["spin"][js][v] ~= nil then
                    observed["spin"][js][v] = observed["spin"][js][v] + 1
                else
                    observed["spin"][js][v] = 1
                end

                -- flop
                x = cohort.cohort_flop(i, cs, seed)
                rc = cohort.cohort_flop(x, cs, seed)
                assert(
                    i == rc,
                    "flop(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["flop"][js][v] ~= nil then
                    observed["flop"][js][v] = observed["flop"][js][v] + 1
                else
                    observed["flop"][js][v] = 1
                end

                -- mix
                x = cohort.cohort_mix(i, cs, seed)
                rc = cohort.rev_cohort_mix(x, cs, seed)
                assert(
                    i == rc,
                    "mix(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["mix"][js][v] ~= nil then
                    observed["mix"][js][v] = observed["mix"][js][v] + 1
                else
                    observed["mix"][js][v] = 1
                end

                -- spread
                x = cohort.cohort_spread(i, cs, seed)
                rc = cohort.rev_cohort_spread(x, cs, seed)
                assert(
                    i == rc,
                    "spread(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["spread"][js][v] ~= nil then
                    observed["spread"][js][v] = observed["spread"][js][v] + 1
                else
                    observed["spread"][js][v] = 1
                end

                -- upend
                x = cohort.cohort_upend(i, cs, seed)
                rc = cohort.cohort_upend(x, cs, seed)
                assert(
                    i == rc,
                    "upend(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["upend"][js][v] ~= nil then
                    observed["upend"][js][v] = observed["upend"][js][v] + 1
                else
                    observed["upend"][js][v] = 1
                end

                -- shuffle
                x = cohort.cohort_shuffle(i, cs, seed)
                rc = cohort.rev_cohort_shuffle(x, cs, seed)
                assert(
                    i == rc,
                    "shuffle(" .. i .. ", " .. cs .. ", " .. seed .. ")→"
                 .. x .. "→" .. rc .. ""
                )

                v = tostring(x)
                if observed["shuffle"][js][v] ~= nil then
                    observed["shuffle"][js][v] = observed["shuffle"][js][v] + 1
                else
                    observed["shuffle"][js][v] = 1
                end
                progress()
            end
        end

        for _, prp in ipairs(observed) do
            if prp == "interleave" then
                for i = 1,cs do
                    v = tostring(i)
                    if observed[prp][v] ~= nil then
                        count = observed[prp][v]
                    else
                        count = 0
                    end

                    assert(
                        count == 1,
                        "" .. prp .. "(" .. i .. ", " .. cs .. ") found "
                     .. count
                    )
                    progress()
                end

            else
                for j, seed in ipairs(TEST_VALUES) do
                    k = tostring(j)
                    for i = 1,cs do
                        v = tostring(i)
                        count = observed[prp][k][v] or 0
                        assert(
                            count == 1,
                            "" .. prp .. "(" .. i .. ", " .. cs .. ", "
                         .. seed .. ") found " .. count
                        )
                        progress()
                    end
                end
            end
        end
    end
end
test_cohort_ops()


function test_distributions()
    -- Tests the distribution operations.
    dist_setups = {
        -- items, segments, capacity
        {10, 5, 4},  -- 10 items across 5 buckets of 4 items each
        {100, 5, 20},  -- 100 items across 5 buckets of 20 items each
        {100, 1000, 1},  -- 100 items across 1000 1-item buckets
        {100, 1000, 2},  -- 100 items across 1000 2-item buckets
        {100, 100, 10},  -- 100 items across 100 10-item buckets
        {100, 3, 50},  -- 100 items across 3 50-item buckets
        {100, 4, 25},  -- 100 items across 4 25-item buckets
    }
    start_progress(
        "smooth_dist_split_points",
        3 * #dist_setups
    )
    for _, setup in ipairs(dist_setups) do
        local total, segments, capacity = unpack(setup)
        for _, seed in ipairs({8349, 0, 1029104024}) do
            local split, half = distribution.distribution_spilt_point(
                total,
                segments,
                capacity,
                0,
                seed
            )
            assert(
                half == floor(segments / 2),
                string.format(
                    (
                        "bad half %d ~= %d for seed %d setup"
                     .. " %d total across %d size-%d segments"
                    ),
                    half, floor(segments / 2), seed,
                    total, segments, capacity
                )
            )
            local exp_split = floor(total * floor(segments / 2) / segments)
            assert(
                split == exp_split,
                string.format(
                    (
                        "bad split %d ~= %d for seed %d setup"
                     .. " %d total across %d size-%d segments"
                    ),
                    split, exp_split, seed,
                    total, segments, capacity
                )
            )
            progress()
        end
    end

    start_progress(
        "rough_dist_split_points",
        1000 * #dist_setups
    )
    local roughness = 0.75
    for _, setup in ipairs(dist_setups) do
        local total, segments, capacity = unpack(setup)
        local splits = {}
        local first_half = floor(segments / 2)
        -- Natural portion for first half of segments
        local nat = floor(total * first_half / segments)
        -- Min pre-split and max post-split capacities given the total
        local pre_cap = first_half * capacity
        local post_cap = (segments - first_half) * capacity

        -- Min portion given roughness
        local min_portion = floor((1 - roughness) * nat)
        if min_portion < total - post_cap then
            min_portion = total - post_cap
        end
        -- Max portion given roughness
        local max_portion = floor(nat + (total - nat) * roughness)
        if max_portion > pre_cap then
            max_portion = pre_cap
        end
        for i = 0,1000 do
            local seed = 74932 + i * 398
            split, half = distribution.distribution_spilt_point(
                total,
                segments,
                capacity,
                roughness,
                seed
            )
            assert(
                half == first_half,
                string.format(
                    (
                        "bad half %d ~= %d for seed %d setup"
                     .. " %d total across %d size-%d segments"
                    ),
                    half, first_half, seed,
                    total, segments, capacity
                )
            )
            assert(
                min_portion <= split and split <= max_portion,
                string.format(
                    (
                        "bad split BROKE %d <= %d <= %d for seed %d setup"
                     .. " %d total across %d size-%d segments"
                    ),
                    min_portion, split, max_portion, seed,
                    total, segments, capacity
                )
            )
            splits[#splits+1] = split
            progress()
        end

        -- Test for variability & coverage of splits
        if min_portion < max_portion then
            local fd = false
            local fs = splits[1]
            local fmin = fs
            local fmax = fs
            for _, s in ipairs(splits) do
                if s ~= fs then
                    fd = true
                end
                if s < fmin then
                    fmin = s
                end
                if s > fmax then
                    fmax = s
                end
            end
            assert(
                fd,
                string.format(
                    (
                        "NO split variability (all %d) despite min"
                     .. " %d and max %d) for setup"
                     .. " %d total across %d size-%d segments"
                    ),
                    fs,
                    min_portion, max_portion,
                    total, segments, capacity
                )
            )

            local coverage = (fmax - fmin) / (max_portion - min_portion)
            assert(
                coverage > 0.5,
                string.format(
                    (
                        "SUSPICIOUSLY small coverage of available"
                     .. " splits: %.3f < 0.5 over 1000 seeds. Limits"
                     .. " %d to %d but only covered %d to %d for setup"
                     .. " %d total across %d size-%d segments"
                    ),
                    coverage,
                    min_portion, max_portion, fmin, fmax,
                    total, segments, capacity
                )
            )
        end
    end

    local total_segments = 0
    for _, setup in ipairs(dist_setups) do
        local total, segments, capacity = unpack(setup)
        total_segments = total_segments + segments
    end
    start_progress(
        "dist_portions",
        20 * total_segments
    )
    for _, setup in ipairs(dist_setups) do
        local total, segments, capacity = unpack(setup)
        splits = {}
        -- Natural portion for each segment with 0 roughness
        local per_segment = floor(total / segments)
        -- Few seeds w/ 0 roughness
        for i = 1,10 do
            seed = 123948 + 2398*i
            local sofar = 0
            for s = 1,segments do
                local start, portion = distribution.distribution_items(
                    s,
                    total,
                    segments,
                    capacity,
                    0,
                    seed
                )
                local prior = start - 1
                assert(
                    prior == sofar,
                    string.format(
                        (
                            "Prior sum mismatch for segment %d: counted"
                         .. " %d but computed %d for seed %d and setup"
                         .. " %d total across %d size-%d segments"
                        ),
                        s,
                        sofar, prior, seed,
                        total, segments, capacity
                    )
                )
                assert(
                    portion == per_segment or portion == per_segment + 1,
                    string.format(
                        (
                            "Invalid portion %d for segment %d with 0"
                         .. "roughness; should have been either %d or %d"
                         .. " for each segment. Seed %d and setup"
                         .. " %d total across %d size-%d segments."
                        ),
                        portion, s,
                        per_segment, per_segment + 1,
                        seed,
                        total, segments, capacity
                    )
                )
                sofar = sofar + portion
                progress()
            end
            assert(
                sofar == total,
                string.format(
                    (
                        "Total sum mismatch: counted %d but total was %d"
                     .. " for seed %d and setup %d total across %d size-%d"
                     .. " segments."
                    ),
                    sofar, total,
                    seed, total, segments, capacity
                )
            )
        end
            
        -- Once more with roughness
        for i = 1,10 do
            seed = 3748 + 273*i
            sofar = 0
            for s = 1,segments do
                start, portion = distribution.distribution_items(
                    s,
                    total,
                    segments,
                    capacity,
                    roughness,
                    seed
                )
                local prior = start - 1
                assert(
                    prior == sofar,
                    string.format(
                        (
                            "Prior sum mismatch for segment %d: counted"
                         .. " %d but computed %d for seed %d and setup"
                         .. " %d total across %d size-%d rough segments"
                        ),
                        s,
                        sofar, prior, seed,
                        total, segments, capacity
                    )
                )
                assert(
                    portion <= capacity,
                    string.format(
                        (
                            "Portion %d too large for segment %d with %.2f"
                         .. "roughness. Seed %d and setup"
                         .. " %d total across %d size-%d segments."
                        ),
                        portion, s, roughness,
                        seed,
                        total, segments, capacity
                    )
                )
                sofar = sofar + portion
                progress()
            end
            assert(
                sofar == total,
                string.format(
                    (
                        "Total sum mismatch: counted %d but total was %d"
                     .. " for seed %d and setup %d total across %d size-%d"
                     .. " segments with %.2f roughness."
                    ),
                    sofar, total,
                    seed, total, segments, capacity, roughness
                )
            )
        end
    end
end
test_distributions()


function test_sumtables()
    -- Tests sumtable functions from `anarchy.distribution`.
    local p = {2, 2, 2, 2, 2}
    local t = {2, 4, 6, 8, 10}
    local total = 10
    local n_segments = 5

    -- Error on empty sum table
    local status, res = pcall(distribution.sumtable_segment, {}, 0)
    assert(status == false)

    -- Error on negative index
    local status, res = pcall(distribution.sumtable_segment, t, -3)
    assert(status == false)

    -- Error on too-large index
    local status, res = pcall(distribution.sumtable_segment, t, 11)
    assert(status == false)
    local status, res = pcall(distribution.sumtable_segment, t, 100)
    assert(status == false)

    for _, seed in ipairs(TEST_VALUES) do
        local nt = distribution.sumtable_for_distribution(
            total,
            n_segments,
            10,  -- not limiting
            0,  -- distribution should be even
            seed  -- shouldn't matter
        )
        for i, e in ipairs(nt) do
            assert(e == t[i], "smooth @" .. i .. " " .. e .. " ~= " .. t[i])
        end
        local ntr = distribution.sumtable_for_distribution(
            total,
            n_segments,
            2,
            1,  -- doesn't matter since it's limited by capacity
            seed  -- shouldn't matter
        )
        for i, e in ipairs(ntr) do
            assert(e == t[i], "rough @" .. i .. " " .. e .. " ~= " .. t[i])
        end
        -- Shorter by 1 should still work
        snt = distribution.sumtable_for_distribution(
            total - 2,
            n_segments - 1,
            10,  -- not limiting
            0,  -- distribution should be even
            seed  -- shouldn't matter
        )
        for i, e in ipairs(snt) do
            assert(
                e == t[i],
                "short smooth @" .. i .. " " .. e .. " ~= " .. t[i]
            )
        end
        sntr = distribution.sumtable_for_distribution(
            total - 2,
            n_segments - 1,
            2,
            1,  -- doesn't matter since it's limited by capacity
            seed  -- shouldn't matter
        )
        for i, e in ipairs(sntr) do
            assert(
                e == t[i],
                "short rough @" .. i .. " " .. e .. " ~= " .. t[i]
            )
        end
    end

    -- Check integrity of two distribution tables
    for _, seed in ipairs({2938, 823974}) do
        total = 384 + seed % 423
        n_segments = 10 + seed % 100
        cap = floor((total * 1.5) / n_segments)
        rough = rng.uniform(seed)
        t = distribution.sumtable_for_distribution(
            total, n_segments, cap, rough, seed
        )
        assert(#t == n_segments)
        for i = 1,#t do
            local dstart, dhere = distribution.distribution_items(
                i,
                total, n_segments, cap, rough, seed
            )
            local tstart, there = distribution.sumtable_items(t, i) 
            assert(
                dstart == tstart,
                (
                    "Segment " .. i .. " starts at " .. dstart
                 .. " according to distribution but " .. tstart
                 .. " according to table."
                )
            )
            assert(
                dhere == there,
                (
                    "Segment " .. i .. " contains " .. dhere
                 .. " according to distribution but " .. there
                 .. " according to table."
                )
            )
        end
        for item = 1,total do
            assert(
                distribution.distribution_segment(
                    item,
                    total, n_segments, cap, rough, seed
                ) == distribution.sumtable_segment(t, item)
            )
        end
    end

    -- Sumtables from portions for a few different hand-crafted examples
    for _, pair in ipairs({
        {{3, 1, 0, 2, 4, 2}, {3, 4, 4, 6, 10, 12}},
        {{100, 900, 9000, 1}, {100, 1000, 10000, 10001}},
        {{3, 0, 0, 0}, {3, 3, 3, 3}},
        {{0, 0, 1, 1, 0, 0, 3}, {0, 0, 1, 2, 2, 2, 5}},
    }) do
        local p, t = unpack(pair)
        local ct = 0
        for _, n in ipairs(p) do
            ct = ct + n
        end
        -- Check table properties
        nt = distribution.sumtable_from_portions(p)
        for i, e in ipairs(nt) do
            assert(e == t[i], "SFP @" .. i .. ": " .. e .. " ~= " .. t[i])
        end
        assert(distribution.sumtable_total(t) == ct)
        assert(distribution.sumtable_segments(t) == #p)
        -- Figure out item -> index mapping empirically
        imap = {}
        sofar = 0
        for i = 1,#p do
            -- Check portion + items here
            assert(distribution.sumtable_portion(t, i) == p[i])
            local start, nhere = distribution.sumtable_items(t, i)
            local nprior = start - 1
            assert(nprior == sofar)
            assert(nhere == p[i])
            for j = start, start + p[i] - 1 do
                imap[j] = i
            end
            sofar = sofar + p[i]
        end

        -- Check empirical map of item -> index
        for item, in_segment in pairs(imap) do
            assert(distribution.sumtable_segment(t, item) == in_segment)
        end
    end
end
test_sumtables()


-- This is a long test so we save it for last
function test_prng_cycles()
    -- Test lots of different seeds/starts looking for too-short cycles
    start_progress("prng cycles", 292 * 2839 * 50)
    for s = 1,292 do
        local seed = 2983 + 849843 * s
        for st = 1,2839 do
            local start = 4 + st * 29839
            local n = start
            for i = 1,50 do
                n = rng.prng(n, seed)
                assert(
                    n ~= start,
                    (
                        "Found cycle of length " .. i .. " for seed " .. seed
                     .. " starting from " .. start
                    )
                )
                progress()
            end
        end
    end

    -- Compute full cycle lengths (up to a limit) for fewer seeds/starts
    start_progress("prng cycle lengths", 8 * 100 * 50)
    for s = 1,8 do
        local seed = rng.scramble_seed(s)
        for st = 1,100 do
            local start = rng.rev_prng(st, seed + 8238)
            local n = start
            local clen = nil
            for i = 1,10000 do
                n = rng.prng(n, seed)
                if n == start then
                    clen = i
                    break
                end
            end
            -- Print analysis
            if clen == nil then
                print(
                    "Cycle length for seed " .. seed .. " starting from "
                 .. start .. " is at least 10000."
                )
            else
                print(
                    "Cycle length for seed " .. seed .. " starting from "
                 .. start .. " is " .. clen
                )
            end
        end
    end
end
test_prng_cycles()
