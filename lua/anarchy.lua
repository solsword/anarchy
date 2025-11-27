--- anarchy.lua
-- @module anarchy
-- Reversible chaos library (version for Lua 5.3+).
-- @author Peter Mawhorter
-- @copyright 2025 Peter Mawhorter
-- @release 1.0-1
--
-- This version uses the native integers available in Lua versions 5.3+,
-- which are 64-bit integers by default or 32-bit integers if your Lua
-- was compiled with only 32-bit integer support.
--
--  If you want to use Lua 5.1, 5.2 or LuaJIT, use the 'anarchy32'
--  library instead, although performance and probably quality will be
--  worse.

-- Local refs for math functions
local min = math.min
local log = math.log
local floor = math.floor
local ceil = math.ceil

-- Determine number of ID bytes by packing a single integer and measuring
-- the number of bytes in the result.
local ID_BYTES = #string.pack("J", 0)
local ID_BITS = ID_BYTES * 8
local LEFTMOST_BIT = 1 << (ID_BITS - 1)

-- Set up module table
local anarchy = {
    version = 1.0,
    ID_BITS = ID_BITS,
    ID_BYTES = ID_BYTES,
    LEFTMOST_BIT = LEFTMOST_BIT,
    util = {},
    rng = {},
    cohort = {},
    distribution = {},
}

--- Modulus that's always positive
-- Use % operator instead in Lua since it has the same behavior
-- Included for compatibility with anarchy implementations in other
-- languages.
-- @tparam number num The number to compute the modulus of.
-- @tparam number base The base of the modulus.
-- @treturn number The number modulo the base.
local posmod = function (num, base)
    return num % base
end
anarchy.util.posmod = posmod

--- A string hashing function.
-- See: https://stackoverflow.com/questions/7616461/generate-a-hash-from-string-in-javascript-jquery
-- @tparam string s The string to hash
-- @treturn number The hash for that string
local hash_string = function(s)
    local hash = 0
    local i
    local chr
    if #s == 0 then
        return hash
    end
    for i = 1,#s do
        chr = s:byte(i)
        hash = (hash * 31) + chr
    end
    return hash
end
anarchy.util.hash_string = hash_string

--- Creates a mask with the given number of 1 bits, up to
-- `anarchy.ID_BITS` maximum (any larger value gives all-1s, same as if
-- you had given `anarchy.ID_BITS`).
-- @tparam number bits The number of 1 bits to put in the mask.
-- @treturn number A value with that many 1 bits starting from the
--     least significant place.
local mask = function(bits)
    return (1 << bits) - 1
end
anarchy.util.mask = mask

--- Returns a mask that covers just the nth byte (zero-indexed), starting
-- from the least-significant digits.
-- Returns 0 if n is >= `anarchy.ID_BYTES`.
-- @tparam number n The byte index (starting from 0)
-- @treturn number The mask with 1s in that byte and 0s elsewhere
local byte_mask = function(n)
    return (1 << (n*8)) * 255
end
anarchy.util.byte_mask = byte_mask

--- Circular bit shift to the right
-- Distance is capped at 3/4 of ID_BITS
-- @paramt number x The number to swirl.
-- @paramt number distance How far to swirl it.
-- @returnt number The swirled number.
local swirl = function(x, distance)
    distance = distance % floor(3 * ID_BITS / 4)
    local m = mask(distance)
    local fall_off = x & m
    local shift_by = (ID_BITS - distance)
    return (
        (x >> distance)
      | (fall_off << shift_by)
    )
end
anarchy.rng.swirl = swirl

--- Circular bit shift to the left inverse of swirl
-- Distance is capped at 3/4 of ID_BITS
-- @paramt number x The number to unswirl.
-- @paramt number distance How far to unswirl it.
-- @returnt number The unswirled number.
local rev_swirl = function(x, distance)
    distance = distance % floor(3 * ID_BITS / 4)
    local rest = ID_BITS - distance
    local edge = mask(distance) << rest
    local around = (x & edge) >> rest
    return x << distance | around
end
anarchy.rng.rev_swirl = rev_swirl

--- Folds lower bits into upper bits using xor.
-- fold is its own inverse.
-- 'where' is restricted to fall between 1/4 and 1/2 of ID_BITS.
-- @paramt number x the number to fold.
-- @paramt number where which tit to fold at
local fold = function(x, where)
    local quarter = floor(ID_BITS / 4);
    where = where % quarter + quarter
    local m = mask(where)
    local lower = x & m
    local shift_by = ID_BITS - where
    return x ~ (lower << shift_by)
end
anarchy.rng.fold = fold


-- Left half of each byte; used for flopping by swapping these bits with
-- the right half of each byte.
local FLOP_MASK = 0xf0
-- Extend byte-by-byte until we hit max # of bytes (detected when a 1
-- shows up in the leftmost bit).
while (FLOP_MASK & LEFTMOST_BIT) == 0 do
    FLOP_MASK = (FLOP_MASK << 8) | FLOP_MASK
end
anarchy.rng.FLOP_MASK = FLOP_MASK


--- Flops each 1/2 byte with the adjacent 1/2 byte.
-- flop is its own inverse.
-- @paramt number x the number to flop
-- @returnt number The flopped number
local flop = function(x)
    local left = x & FLOP_MASK
    local right = x & ~FLOP_MASK
    return (left >> 4) | (right << 4)
end
anarchy.rng.flop = flop


-- Note that none of the bits in the SCRAMBLE_TRIGGER is adjacent to a
-- bit in the SCRAMBLE_SET. This allows us to check even after
-- scrambling whether the trigger would have activated beforehand.
local SCRAMBLE_TRIGGER = 0x80200003
local SCRAMBLE_SET = 0x03040610
-- Extend one byte at a time to fill available bit width by copying 4th
-- byte into 1st position. So for 8 bytes we'll get 2 copies of this
-- pattern. This may fail i
if ID_BITS % 8 == 0 then
    while SCRAMBLE_TRIGGER & LEFTMOST_BIT == 0 do
        SCRAMBLE_TRIGGER = (
            ((SCRAMBLE_TRIGGER >> 24) & 0xff)
          | (SCRAMBLE_TRIGGER << 8)
        )
        SCRAMBLE_SET = (
            ((SCRAMBLE_SET >> 24) & 0xff)
          | (SCRAMBLE_SET << 8)
        )
    end
end
anarchy.rng.SCRAMBLE_TRIGGER = SCRAMBLE_TRIGGER
anarchy.rng.SCRAMBLE_SET = SCRAMBLE_SET

--- Implements a reversible linear-feedback-shift-register-like operation.
-- @paramt number x the number to scramble.
-- @returnt number the scrambled number
local scramble = function(x)
    -- trigger if we have a 1 in one of several specific bits
    local trigger = ((x & SCRAMBLE_TRIGGER) ~= 0)
    -- circular shift right
    local fall = x & 1
    local r = (x >> 1) | (fall << (ID_BITS - 1))
    -- XOR with constant if we hit a trigger bit, adding/changing bits
    if trigger then
        -- Note: This constant has no bits set next to bits of our
        -- trigger mask, so that even after this operation, if we
        -- left-shift by 1, the trigger boolean will get the same value.
        -- This is key to reversibility.
        r = r ~ SCRAMBLE_SET
    end
    return r
end
anarchy.rng.scramble = scramble

--- Inverse of scramble (see above).
-- @paramt number x the number to unscramble
-- @returnt number the unscrambled number
local rev_scramble = function(x)
    local around = (x & LEFTMOST_BIT) >> (ID_BITS - 1)
    local pr = x << 1 | around
    -- Same trigger mask as in scramble, because we know that scramble
    -- didn't mess with any of the bits the trigger relies on.
    local trigger = ((pr & SCRAMBLE_TRIGGER) ~= 0)
    if trigger then
        pr = pr ~ (SCRAMBLE_SET << 1)
    end
    return pr
end
anarchy.rng.rev_scramble = rev_scramble


--- Scrambles a seed value to help separate RNG sequences generated from
-- sequential seeds.
-- @paramt number s the seed to scramble
-- @returnt number the scrambled seed
local scramble_seed = function(s)
    s = (s + 1) * (3 + s % 23)
    s = fold(s, 11) -- prime
    s = scramble(s)
    s = swirl(s, s + 23) -- prime
    s = scramble(s)
    s = s ~ ((s % 153) * scramble(s))

    return s;
end
anarchy.rng.scramble_seed = scramble_seed

--- A simple reversible pseudo-random number generator
-- You can call @see anarchy.rng.rev_prng with the result of this
-- function and the same seed to get back the number you put in here,
-- so that results form a chain you can traverse forwards or backwards.
-- @paramt number x the current/previous number
-- @paramt number seed the generation sequence seed (this can be
--     constant for a bunch of calls)
-- @returnt number the next number in the generation sequence
local prng = function(x, seed)

    -- seed scrambling:
    seed = scramble_seed(seed)

    -- value scrambling:
    x = x ~ seed
    x = scramble(x)
    x = fold(x, seed + 17) -- prime
    x = flop(x)
    x = swirl(x, seed + 37) -- prime
    x = fold(x, seed + 89) -- prime
    x = swirl(x, seed + 107) -- prime
    x = scramble(x)
    return x
end
anarchy.rng.prng = prng

--- Inverse of @see anarchy.rng.prng (see above).
-- @paramt number x the current/next number
-- @paramt number seed the generation seed that determines the sequence
-- @returnt number the previous number in the generation sequence
local rev_prng = function(x, seed)

    -- seed scrambling:
    seed = scramble_seed(seed)

    -- value unscrambling:
    x = rev_scramble(x)
    x = rev_swirl(x, seed + 107)  -- prime
    x = fold(x, seed + 89)  -- prime
    x = rev_swirl(x, seed + 37)  -- prime
    x = flop(x)
    x = fold(x, seed + 17)  -- prime
    -- TODO: This extra scramble helps pass flip tests but makes cycle
    -- tests worse...
    x = rev_scramble(x)
    x = x ~ seed
    return x
end
anarchy.rng.rev_prng = rev_prng

-- For 32-bit version, same as the scramble trigger we use bits 32, 22,
-- 2, and 1. See this PDF table of max-length LFSR parameters:
-- http://courses.cse.tamu.edu/walker/csce680/lfsr_table.pdf
local LFSR_BITS = 0x80200003
if ID_BYTES == 8 then
    -- For 64-bit version, we use bits 64, 63, 61, and 60.
    LFSR_BITS = 0xe800000000000000
end
-- We won't worry here about bit widths other than 32 or 64, which should
-- be the only two ones we'll see. Other bit widths might not be
-- maximum-length cycles given these values.
anarchy.rng.LFSR_BITS = LFSR_BITS


--- Implements a max-cycle-length 32-bit linear feedback shift register.
-- See: https://en.wikipedia.org/wiki/Linear-feedback_shift_register
-- Note that this is NOT reversible!
-- Note: Do not use this as an irreversible PRNG; it's a terrible one!
-- Note: Zero is a fixed point of this function: do not use it as
-- your seed!
-- @paramt number x the current number
-- @returnt number the next number in the LFSR sequence
local lfsr = function(x)
    -- Note: the SCRAMBLE_TRIGGER has bits 32, 22, 2, 1 set, or if we're
    -- using 64-bit integers, bits 64, 54, 34, 33, 32, 22, 2, and 1.
    return (x >> 1) ~ (LFSR_BITS * (x & 1))
end
anarchy.rng.lfsr = lfsr


-- Note: We set the sign bit for U_DIVISOR in both cases to 0 to avoid
-- any unexpected surprises about signed vs. unsigned values.
local U_DIVISOR = 0x7fffffff  -- largest prime below 2^31 is 2^31 - 1
local U_SHIFT = 16  -- half the bits
if ID_BYTES == 8 then
    U_DIVISOR = 0x7fffffffffffffe7 -- largest prime below 2^63
    U_SHIFT = 32
end
anarchy.rng.U_DIVISOR = U_DIVISOR

--- Generates a random number between 0 and 1 given a seed value.
-- @paramt number seed The seed that determines the result.
-- @returnt number A number between 0 (inclusive) and 1 (exclusive)
local uniform = function(seed)
    local sc = seed ~ (seed << U_SHIFT)
    local ux = prng(prng(prng(sc, 53), sc), seed)
    return (ux % U_DIVISOR) / U_DIVISOR
end
anarchy.rng.uniform = uniform

--- Generates and averages three random numbers between 0 and 1 to give a
-- pseudo-gaussian-distributed random number (still strictly on [0, 1) )
-- @paramt number seed The seed that determines the result.
-- @returnt number A number between 0 (inclusive) and 1 (exclusive)
--     which is more likely to be near 0.5 than near either 0 or 1.
local normalish = function(seed)
    local t = 0
    t = t + uniform(seed + 9182183)
    t = t + uniform(seed + 7438742)
    t = t + uniform(seed + 4683129)
    return t/3
end
anarchy.rng.normalish = normalish

--- Flips a coin with probability p of being True. Using the same seed
-- always gives the same result.
-- @paramt number p The probability that the result should be True when
--     results are averaged across many different seeds.
-- @paramt number seed The seed that determines the result.
-- @returnt boolean Either true (with probability p when averaged
--     across seeds) or false (with probability 1-p when averaged
--     across seeds).
local flip = function(p, seed)
    return uniform(seed) < p
end
anarchy.rng.flip = flip

--- Even distribution over the given integer range, including the start
-- but excluding the cap (even if the cap is lower than the start).
-- The last number before the cap is about 1/`anarchy.rng.U_DIVISOR` less
-- likely to be generated than the other numbers.
--
-- Note: Why route integer stuff through uniform instead of just using a
-- modulus which would be faster? Because we have little idea how
-- selective biased lfsr and/or prng values might be for an arbitrary
-- modulus. Our prng is almost certainly NOT capable of producing every
-- 64-bit number, for example, and might even have some short cycles out
-- there somewhere.
--
-- @paramt number seed The seed that determines the result.
-- @paramt number start The minimum allowed result value.
-- @paramt number cap The result limit (will NOT ever be generated itself).
-- @returnt number An integer between start (inclusive) and cap (exclusive).
local integer = function(seed, start, cap)
    return floor(uniform(seed) * (cap - start)) + start;
end
anarchy.rng.integer = integer

--- Generates a number from an exponential distribution on [0,∞) with mean
-- 1/lambda given a seed. Higher values of lambda make the distribution more
-- sharply exponential; values between 0.5 and 1.5 exhibit reasonable
-- variation. See:
-- https://math.stackexchange.com/questions/28004/random-exponential-like-distribution
-- and
-- https://en.wikipedia.org/wiki/Exponential_distribution
-- @paramt number seed The seed that determines the result.
-- @paramt number lambda The shape parameter for the distribution.
-- @returnt number An arbitrarily large number (subject to the limits of the
--     number type) that's much more likely to be small than large.
local exponential = function(seed, lambda)
    local u = uniform(seed)
    return -log(u)/lambda
end
anarchy.rng.exponential = exponential

--- Generates a number from a truncated exponential distribution on [0,
-- 1), given a particular seed. As with expdist, the lambda parameter
-- controls the shape of the distribution, and a 0.5–1.5 range is usually
-- reasonable. For why this method works, see:
-- https://math.stackexchange.com/questions/28004/random-exponential-like-distribution
-- @paramt number seed The seed that determines the result.
-- @paramt number lambda The shape parameter for the distribution.
-- @returnt number A number between 0 (inclusive) and 1 (exclusive) that
--     is much more likely to be small than large.
local truncated_exponential = function(seed, lambda)
    local e = exponential(seed, lambda);
    return e - floor(e);
end
anarchy.rng.truncated_exponential = truncated_exponential


--- Computes cohort number for the given outer index and cohort size.
-- @paramt number outer An index into a sequence to be chopped into
--     cohorts, starting with the beginning of the first cohort at index 1.
-- @paramt number cohort_size The size of each cohort to divide the
--     sequence into.
-- @returnt number The index of the cohort that the outer index falls
--     into. The first cohort (starting at index 1 of the outer sequence)
--     has index 1.
--
-- For example, if outer index is 54 and cohort_size is 10, the return
-- value will be 6, since 54 falls into the 6th group of 10.
local cohort = function(outer, cohort_size)
    return floor((outer - 1) / cohort_size) + 1
end
anarchy.cohort.cohort = cohort

--- Computes within-cohort index for the given outer index and cohorts of
-- the given size.
-- @paramt number outer An index into a sequence to be chopped into
--     cohorts, starting with the beginning of the first cohort at index 1.
-- @paramt number cohort_size The size of each cohort to divide the
--     sequence into.
-- @returnt number The index of the given outer index within its cohort,
--     starting from 1 in the first position.
--
-- For example, if the outer index is 54 and the cohort_size is 10, the
-- result will be 4, since 54 falls at index 4 within its group of 10
-- (starting at index 1 at 51).
local cohort_inner = function(outer, cohort_size)
    return 1 + ((outer - 1) % cohort_size)
end
anarchy.cohort.cohort_inner = cohort_inner

--- Returns two results: the cohort number and inner index for the given
-- outer index and cohort size.
-- @paramt number outer An index into a sequence to be chopped into cohorts.
-- @paramt number cohort_size The size of each cohort to divide the
--     sequence into.
-- @return Multiple values: The cohort index (an integer) followed by the
--     inner index within that cohort (also an integer).
--
-- For example, if the outer index is 29 and the cohort_size is 10, this will
-- return 3, 9
local cohort_and_inner = function(outer, cohort_size)
    return cohort(outer, cohort_size), cohort_inner(outer, cohort_size)
end
anarchy.cohort.cohort_and_inner = cohort_and_inner

--- Inverse of cohort_and_inner; computes the outer index from a cohort
-- number and inner index.
-- @paramt number cohort The cohort index determining which group a position
--     belongs to.
-- @paramt number inner The index of the position within that group.
--     Should be between 1 and the cohort size, inclusive.
-- @paramt number cohort_size The size of each group to divide the total
--     sequence of positions into.
-- @returnt number The index of that position within the total sequence of
--     positions, starting from index 1.
--
-- For example, if cohort is 3, inner is 1, and cohort_size is 5, then
-- we're at index 1 (1st item) within the 3th group of 5 things, putting
-- us at total index 5*2 + 1 = 11 outside of the groups.
--
-- Note: Negative cohort and/or inner indices may result in negative
-- outer indices. The inner index value is not constrained to be smaller
-- than the cohort size.
local cohort_outer = function(cohort, inner, cohort_size)
    return cohort_size * (cohort - 1) + inner
end
anarchy.cohort.cohort_outer = cohort_outer

--- Interleaves cohort members by folding the top half into the bottom
-- half. The bottom half get even indices while the top half are assigned
-- backwards to the odd indices.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @returnt number The new within-cohort index for the same item after
--     interleaving the top and bottom parts of the cohort.
local cohort_interleave = function(inner, cohort_size)
    if inner <= floor(cohort_size/2) then
        return inner * 2  -- even and never > cohort_size
    else
        return ((cohort_size - inner) * 2) + 1
    end
end
anarchy.cohort.cohort_interleave = cohort_interleave

--- Inverse interleave (see above).
-- @paramt number inner The index of an item within a cohort after
--     interleaving items.
-- @paramt number cohort_size The size of each cohort.
-- @returnt number The original index within that cohort that the item
--     would have been at prior to interleaving things.
local rev_cohort_interleave = function(inner, cohort_size)
    if inner % 2 ~= 0 then
        return cohort_size - floor(inner/2)
    else
        return floor(inner / 2)
    end
end
anarchy.cohort.rev_cohort_interleave = rev_cohort_interleave


--- Folds items past an arbitrary split point (in the second half of the
-- cohort) into the middle of the cohort. The split will always leave an
-- odd number at the end.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines where the fold happens.
-- @returnt number The new index of the item within the cohort after folding.
local cohort_fold = function(inner, cohort_size, seed)
    local half = ceil(cohort_size / 2)
    local quarter = ceil(cohort_size / 4)
    local split = half
    if quarter > 0 then
        split = split + seed % quarter
    end
    local after = cohort_size - split
    split = split + ((after + 1) % 2)  -- force odd leftovers
    after = cohort_size - split

    local fold_to = half - floor(after / 2)

    if inner < fold_to then  -- first region
        return inner
    elseif inner <= split then  -- second region
        return inner + after  -- push out past fold region
    else  -- fold region
        return (inner - split) + fold_to - 1
    end
end
anarchy.cohort.cohort_fold = cohort_fold

--- Inverse fold (see above).
-- @paramt number inner The index of an item within a cohort after folding.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines where the fold happened.
-- @returnt number The original index that would have been folded into
--     the given position if cohort_fold were used with the same seed..
local rev_cohort_fold = function(inner, cohort_size, seed)
    local half = ceil(cohort_size / 2)
    local quarter = ceil(cohort_size / 4)
    local split = half
    if quarter > 0 then
        split = split + seed % quarter
    end
    local after = cohort_size - split
    split = split + ((after + 1) % 2)  -- force an odd split point
    after = cohort_size - split

    local fold_to = half - floor(after / 2)

    if inner < fold_to then  -- first region
        return inner
    elseif (inner < fold_to + after) then  -- second region
        return inner - (fold_to - 1) + split
    else
        return inner - after
    end
end
anarchy.cohort.rev_cohort_fold = rev_cohort_fold

--- Applies a circular offset
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines how far to spin.
-- @returnt number The index of the item after spinning.
local cohort_spin = function(inner, cohort_size, seed)
    return ((inner - 1) + (seed % cohort_size)) % cohort_size + 1
end
anarchy.cohort.cohort_spin = cohort_spin

--- Inverse spin (see above).
-- @paramt number inner The index of an item within a cohort after being spun.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determined how far to spin.
-- @returnt number The original index of the item before spinning.
local rev_cohort_spin = function(inner, cohort_size, seed)
    return (
        (inner - 1) + (cohort_size - (seed % cohort_size))
    ) % cohort_size + 1
end
anarchy.cohort.rev_cohort_spin = rev_cohort_spin

--- Flops sections (with seeded sizes) with their neighbors.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines how big each section to
--     flop is.
-- @returnt number The index of the item after flopping with a neighboring
--     sesction.
local cohort_flop = function(inner, cohort_size, seed)
    local limit = floor(cohort_size / 8)
    if limit < 4 then
        limit = limit + 4
    end
    local size = (seed % limit) + 2
    local which = floor((inner - 1) / size)
    local in_part = (inner - 1) % size

    local result = 0
    if which % 2 ~= 0 then
        result = (which - 1) * size + in_part + 1
    else
        result = (which + 1) * size + in_part + 1
    end

    if result > cohort_size then  -- don't flop out of the cohort
        return inner
    else
        return result
    end
end
anarchy.cohort.cohort_flop = cohort_flop
-- flop is its own inverse

--- Applies a spin to even and odd items separately with different seeds.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines how far to spin evens & odds.
-- @returnt number The index of the item after spinning the even & odd elements
--     among themselves with different (derived) seeds.
local cohort_mix = function(inner, cohort_size, seed)
    local even = inner - inner % 2
    local target
    if inner % 2 ~= 0 then
        target = cohort_spin(
            1 + floor(even / 2),
            cohort_size - floor(cohort_size / 2),
            seed + 464185
        )
        return 2 * target - 1
    else
        target = cohort_spin(
            floor(even / 2),
            floor(cohort_size / 2),
            seed + 1048239
        )
        return 2 * target
    end
end
anarchy.cohort.cohort_mix = cohort_mix

--- Inverse mix (see above).
-- @paramt number inner The index of an item within a cohort after mixing.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determined how far to spin evens & odds.
-- @returnt number The index that this index would have been before mixing
--     with the given seed.
local rev_cohort_mix = function(inner, cohort_size, seed)
    local even = inner - inner % 2
    local target = 0
    if inner % 2 ~= 0 then
        target = rev_cohort_spin(
            1 + floor(even / 2),
            cohort_size - floor(cohort_size / 2),
            seed + 464185
        )
        return 2 * target - 1
    else
        target = rev_cohort_spin(
            floor(even / 2),
            floor(cohort_size / 2),
            seed + 1048239
        );
        return 2 * target
    end
end
anarchy.cohort.rev_cohort_mix = rev_cohort_mix


-- Constants that limit how regions can be sized in cohort_spread.
local MIN_REGION_SIZE = 2;
local MAX_REGION_COUNT = 16;
anarchy.cohort.MIN_REGION_SIZE = MIN_REGION_SIZE;
anarchy.cohort.MAX_REGION_COUNT = MAX_REGION_COUNT;

--- Spreads items out between a random number of different regions within
-- the cohort.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines how many regions we use.
-- @returnt number The index after spreading out indices between random
--     regions.
local cohort_spread = function(inner, cohort_size, seed)
    local min_regions = 2
    if cohort_size < 2 * MIN_REGION_SIZE then
        min_regions = 1
    end
    local max_regions = 1 + floor(cohort_size / MIN_REGION_SIZE)
    local regions = (
        min_regions + (
            (seed % (1 + (max_regions - min_regions)))
          % MAX_REGION_COUNT
        )
    )
    local region_size = floor(cohort_size / regions)
    local leftovers = cohort_size - (regions * region_size)

    local region = (inner - 1) % regions
    local index = floor((inner - 1) / regions) + 1
    if index <= region_size then  -- non-leftovers
        return region * region_size + index + leftovers
    else  -- leftovers go at the front:
        return (inner - 1) - regions * region_size + 1
    end
end
anarchy.cohort.cohort_spread = cohort_spread

--- Inverse spread (see above).
-- @paramt number inner The index of an item within a cohort after spreading.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determined how many regions we use.
-- @returnt number The index that would have been used to generate the provided
--     index with a spread operation and the given seed.
local rev_cohort_spread = function(inner, cohort_size, seed)
    local min_regions = 2
    if cohort_size < 2 * MIN_REGION_SIZE then
        min_regions = 1
    end
    local max_regions = 1 + floor(cohort_size / MIN_REGION_SIZE)
    local regions = (
        min_regions + (
            (seed % (1 + (max_regions - min_regions)))
          % MAX_REGION_COUNT
        )
    );

    local region_size = floor(cohort_size / regions);
    local leftovers = cohort_size - (regions * region_size);

    local index = floor(((inner - 1) - leftovers) / region_size)
    local region = ((inner - 1) - leftovers) % region_size

    if inner <= leftovers then  -- leftovers back to the end:
        return regions * region_size + inner
    else
        return region * regions + 1 + index
    end
end
anarchy.cohort.rev_cohort_spread = rev_cohort_spread

--- Reverses ordering within each of several fragments.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determined how many regions we use.
-- @returnt number The index after upending the order of each region.
local cohort_upend = function(inner, cohort_size, seed)
    local min_regions = 2
    if (cohort_size < 2 * MIN_REGION_SIZE) then
        min_regions = 1
    end
    local max_regions = 1 + floor(cohort_size / MIN_REGION_SIZE)
    local regions = (
        min_regions + (
            (seed % (1 + (max_regions - min_regions)))
          % MAX_REGION_COUNT
        )
    )
    local region_size = floor(cohort_size / regions)

    local region = floor((inner - 1) / region_size)
    local index = ((inner - 1) % region_size) + 1

    local result = (region * region_size) + (region_size + 1 - index)

    if result <= cohort_size then
        return result
    else
        return inner
    end
end
anarchy.cohort.cohort_upend = cohort_upend
-- Upend is its own inverse.

--- Compose a bunch of the above functions to perform a nice thorough
-- shuffle within a cohort.
-- @paramt number inner The index of an item within a cohort.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determines where to shuffle things.
-- @returnt number The position of the index after shuffling the cohort. When
--     shuffling two different indices with the same cohort size and
--     seed, you'll get two different results. Usually not the same as
--     the original index.
local cohort_shuffle = function(inner, cohort_size, seed)
    local r = inner
    seed = seed ~ cohort_size
    -- print("\ncs", inner, cohort_size)
    r = cohort_spread(r, cohort_size, seed + 457)  -- prime
    -- io.write(" spread " .. r)
    r = cohort_mix(r, cohort_size, seed + 2897)  -- prime
    -- io.write(" mix " .. r)
    r = cohort_interleave(r, cohort_size)
    -- io.write(" interleave " .. r)
    -- io.write("\n")
    r = cohort_spin(r, cohort_size, seed + 1987)  -- prime
    -- io.write(" spin " .. r)
    r = cohort_upend(r, cohort_size, seed + 47)  -- prime
    -- io.write(" upend " .. r)
    r = cohort_fold(r, cohort_size, seed + 839)  -- prime
    -- io.write(" fold " .. r)
    r = cohort_interleave(r, cohort_size)
    -- io.write(" interleave " .. r)
    -- io.write("\n")
    r = cohort_flop(r, cohort_size, seed + 53)  -- prime
    -- io.write(" flop " .. r)
    r = cohort_fold(r, cohort_size, seed + 211)  -- prime
    -- io.write(" fold " .. r)
    r = cohort_mix(r, cohort_size, seed + 733)  -- prime
    -- io.write(" mix " .. r)
    r = cohort_spread(r, cohort_size, seed + 881)  -- prime
    -- io.write(" spread " .. r)
    r = cohort_interleave(r, cohort_size)
    -- io.write(" interleave " .. r)
    -- io.write("\n")
    r = cohort_flop(r, cohort_size, seed + 193)  -- prime
    -- io.write(" flop " .. r)
    r = cohort_upend(r, cohort_size, seed + 794641)  -- prime
    -- io.write(" upend " .. r)
    r = cohort_spin(r, cohort_size, seed + 19)  -- prime
    -- io.write(" spin " .. r)
    -- io.write("\n")
    return r
end
anarchy.cohort.cohort_shuffle = cohort_shuffle

--- Inverse shuffle (see above).
-- @paramt number inner The index of an item within a cohort after shuffling.
-- @paramt number cohort_size The size of each cohort.
-- @paramt number seed The seed that determined where to shuffle things.
-- @returnt number The original index within the cohort that would have been
--     shuffled into the specified position.
local rev_cohort_shuffle = function(inner, cohort_size, seed)
    local r = inner
    seed = seed ~ cohort_size
    -- print("\nrcs", inner, cohort_size)
    r = rev_cohort_spin(r, cohort_size, seed + 19)  -- prime
    -- io.write(" spin " .. r)
    r = cohort_upend(r, cohort_size, seed + 794641)  -- prime
    -- io.write(" upend " .. r)
    r = cohort_flop(r, cohort_size, seed + 193)  -- prime
    -- io.write(" flop " .. r)
    r = rev_cohort_interleave(r, cohort_size)
    -- io.write("\n")
    -- io.write(" interleave " .. r)
    r = rev_cohort_spread(r, cohort_size, seed + 881)  -- prime
    -- io.write(" spread " .. r)
    r = rev_cohort_mix(r, cohort_size, seed + 733)  -- prime
    -- io.write(" mix " .. r)
    r = rev_cohort_fold(r, cohort_size, seed + 211)  -- prime
    -- io.write(" fold " .. r)
    r = cohort_flop(r, cohort_size, seed + 53)  -- prime
    -- io.write(" flop " .. r)
    r = rev_cohort_interleave(r, cohort_size)
    -- io.write("\n")
    -- io.write(" interleave " .. r)
    r = rev_cohort_fold(r, cohort_size, seed + 839)  -- prime
    -- io.write(" fold " .. r)
    r = cohort_upend(r, cohort_size, seed + 47)  -- prime
    -- io.write(" upend " .. r)
    r = rev_cohort_spin(r, cohort_size, seed + 1987)  -- prime
    -- io.write(" spin " .. r)
    r = rev_cohort_interleave(r, cohort_size);
    -- io.write("\n")
    -- io.write(" interleave " .. r)
    r = rev_cohort_mix(r, cohort_size, seed + 2897)  -- prime
    -- io.write(" mix " .. r)
    r = rev_cohort_spread(r, cohort_size, seed + 457)  -- prime
    -- io.write(" spread " .. r)
    -- io.write("\n")
    return r
end
anarchy.cohort.rev_cohort_shuffle = rev_cohort_shuffle


--- Implements common distribution functionality: given a total item
-- count, a segment count and per-segment capacity, and a roughness value
-- and seed, computes the split point for the items as well as the
-- halfway index for the segments.
--
-- Note that per the normal Lua convention, indices for both total items
-- and segments start from 1!
--
-- @paramt number total The total number of items that will be
--     distributed among segments.
-- @paramt number n_segments The number of segments to distribute the
--     items into.
-- @paramt number segment_capacity The capacity of each segment.
--     n_segments times segment_capacity must be >= total.
-- @paramt number roughness A number between 0 and 1. When roughness is
--     0, items will be split across segments as evenly as possible. When
--     roughness is 1, up to 100% of the items may be placed into the
--     first or second half of the segments while recursively
--     apportioning items. At roughness 0.5 the first or second half of
--     the segments might get as little as 50% of what it would get when
--     roughness is 0.
-- @paramt number seed The seed that determines how the total gets
--     distributed among the segments when roughness allows different
--     distributions.
-- @return Multiple results: two numbers. The first is the split point
--     for the distribution: the number out of the total items which end
--     up in the first half of the segments. The second is the number of
--     segments in that first half of the segments, which is half of
--     n_segments, rounded down.
local distribution_spilt_point
distribution_spilt_point = function(
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
)
    -- how the segments are divided:
    local first_half = floor(n_segments / 2)

    -- compute min/max split points according to roughness:
    local nat = floor(total * (first_half / n_segments))
    local split_min = floor(nat - nat * roughness)
    local split_max = floor(nat + (total - nat) * roughness)

    -- adjust for capacity limits:
    if (total - split_min) > segment_capacity * (n_segments - first_half) then
        split_min = total - (segment_capacity * (n_segments - first_half))
    end

    if split_max > segment_capacity * first_half then
        split_max = segment_capacity * first_half
    end

    -- compute a random split point:
    local split = nat
    if split_min >= split_max then
        split = split_min
    else
        split = split_min + (
            prng(total, seed)
          % (split_max - split_min)
        )
    end

    return split, first_half
end
anarchy.distribution.distribution_spilt_point = distribution_spilt_point


--- Given that 'total' items are to be distributed evenly among
-- 'n_segment' segments each with at most 'segment_capacity' items and
-- we're in segment 'segment' of those (indexed from 1), computes the
-- start index (from 1) among the total of the first item in this
-- segment, and how many items are in this segment. The 'roughness'
-- argument should be a number between 0 and 1 indicating how even the
-- distribution is: 0 indicates a perfectly even distribution, while 1
-- indicates a perfectly random distribution. Does work proportional to
-- the log of the number of segments.
--
-- Note that segment_capacity * n_segments should be > total.
--
-- @paramt number segment Which segment we want to know the portion for,
--     as an index starting from 1.
-- @paramt number total The total number of items that will be
--     distributed among segments.
-- @paramt number n_segments The number of segments to distribute the
--     items into.
-- @paramt number segment_capacity The capacity of each segment.
--     n_segments times segment_capacity must be >= total.
-- @paramt number roughness A number between 0 and 1. When roughness is
--     0, items will be split across segments as evenly as possible. When
--     roughness is 1, up to 100% of the items may be placed into the
--     first or second half of the segments while recursively
--     apportioning items. At roughness 0.5 the first or second half of
--     the segments might get as little as 50% of what it would get when
--     roughness is 0.
-- @paramt number seed The seed that determines how the total gets
--     distributed among the segments when roughness allows different
--     distributions.
-- @return Multiple values: the start index (from 1) of the first item in
--     this segment and the number of the total items that get
--     distributed into the specified segment, for the given seed. The
--     number of items May be as low as 0 when roughness is 1; it will be
--     one of the integers adjacent to (total / n_segments) when
--     roughness is 0. If the number of items is 0, then the 'start
--     index' will be the same as the 'start index' for the subsequent
--     segment (and if multiple segments in a row get 0 items they'll all
--     share the same start index with the first segment after them that
--     gets at least 1 item).
local distribution_items
distribution_items = function(
    segment,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
)
    -- base case
    if n_segments == 1 then
        return 1, total
    end

    -- compute split point:
    local split, half = distribution_spilt_point(
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    );

    -- call ourselves recursively:
    if segment <= half then
        return distribution_items(
            segment,
            split,
            half,
            segment_capacity,
            roughness,
            seed
        )
    else
        local start, count = distribution_items(
            segment - half,
            total - split,
            n_segments - half,
            segment_capacity,
            roughness,
            seed
        )
        return split + start, count
    end
end
anarchy.distribution.distribution_items = distribution_items

--- Computes the segment number in which a certain item appears (one of
-- the 'total' items distributed between segments; see distribution_items
-- above). Requires work proportional to the log of the number of
-- segments.
--
-- @paramt number index Which item among the total we want to know the
--     fate of, starting from 1. Must be less than or equal to the total.
-- @paramt number total The total number of items that will be
--     distributed among segments.
-- @paramt number n_segments The number of segments to distribute the
--     items into.
-- @paramt number segment_capacity The capacity of each segment.
--     n_segments times segment_capacity must be >= total.
-- @paramt number roughness A number between 0 and 1. When roughness is
--     0, items will be split across segments as evenly as possible. When
--     roughness is 1, up to 100% of the items may be placed into the
--     first or second half of the segments while recursively
--     apportioning items. At roughness 0.5 the first or second half of
--     the segments might get as little as 50% of what it would get when
--     roughness is 0.
-- @paramt number seed The seed that determines how the total gets
--     distributed among the segments when roughness allows different
--     distributions.
-- @return Multiple values: the segment index (starting from 1) within
--     which the indicated item appears, and then the index within that
--     segment (starting from 1) at which it is placed.
local distribution_segment
distribution_segment = function(
    index,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
)
    -- base case
    if n_segments == 1 then
        return 1, index  -- we are in the only segment there is
    end

    -- compute split point:
    local split, half = distribution_spilt_point(
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )

    -- call ourselves recursively:
    if index <= split then
        return distribution_segment(
            index,
            split,
            half,
            segment_capacity,
            roughness,
            seed
        )
    else
        local segment, within = distribution_segment(
            index - split,
            total - split,
            n_segments - half,
            segment_capacity,
            roughness,
            seed
        )
        return half + segment, within
    end
end
anarchy.distribution.distribution_segment = distribution_segment


-- Forward declaration as local (see function definition below)
local fill_sumtable_for_distribution


--- Takes distribution parameters and returns a table containing the
-- cumulative sum of items in each segment (The table indices will start
-- at 1, just like the segment indices). The list contains `n_segments`
-- entries, and each entry holds the sum of the items from the `total`
-- assigned to that segment plus all earlier segments. This means that
-- each entry is greater than or equal to the entry before it (and
-- they're only equal if zero items are assigned to that segment). The
-- last entry will be equal to the total.
--
-- @paramt number total The total number of items being distributed.
-- @paramt number n_segments The number of segments to distribute items
--     among.
-- @paramt number segment_capacity The capacity of each segment.
-- @paramt number roughness How rough the distribution should be (0-1).
-- @paramt number seed The seed that determines a specific distribution.
--
-- @returnt table An array table holding cumulative sums, starting at
--     index 1 with the number of items in the first segment.
--
-- `n_segments` times `segment_capacity` must be >= `total` otherwise an
-- error will be raised.
local sumtable_for_distribution = function(
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
)

    if n_segments * segment_capacity < total then
        error(
            "Cannot distribute " .. total .. " item(s) across "
         .. n_segments .. " segments of size " .. segment_capacity
         .. ": There's not enough space for them all to fit."
        )
    end

    -- No need to pre-populate table
    result = {}
    fill_sumtable_for_distribution(
        result,
        1,
        0,
        total,
        n_segments,
        segment_capacity,
        roughness,
        seed
    )
    return result
end
anarchy.distribution.sumtable_for_distribution = sumtable_for_distribution


--- Recursively fills in the sumtable based on the distribution
-- parameters given (see `sumtable_for_distribution`). Total work done
-- is `n_segments` times log(`n_segments`).
--
-- Note that using a sumtable instead of using the distribution
-- functions with the same parameters over and over again represents a
-- space/time tradeoff. Most operations on a sumtable are O(1), where
-- equivalent distribution functions need O(log(n)) time. The sumtable
-- needs to store one integer per segment. The use of distribution
-- functions is recommended primarily in situations where the algorithm
-- structure makes it inconvenient to store the sumtable in an
-- accessible way, so sharing parameters is easier.
-- Parameters are the same as for `sumtable_for_distribution` but with:
--
-- @paramt table ledger The table to fill in. Values will be
--     added/overwritten between `start` and `start` + `n_segments` - 1
--     (inclusive).
-- @paramt number start The index at which to start overwriting values
--     (counting from index 1).
-- @paramt number  prior The number of items distributed prior to the start
--     index.
--
-- @returnt nil No return (modifies the given ledger).
fill_sumtable_for_distribution= function(
    table,
    start,
    prior,
    total,
    n_segments,
    segment_capacity,
    roughness,
    seed
)
    if n_segments == 0 then  -- base case: nothing to modify
        assert(total == 0)
        return
    elseif n_segments == 1 then  -- base case: fill in one entry
        table[start] = prior + total
    else  -- recurse into each half of the table
        local sub_prior, first_half = distribution_spilt_point(
            total,
            n_segments,
            segment_capacity,
            roughness,
            seed
        )
        -- Fill in the first half
        fill_sumtable_for_distribution(
            table,
            start,
            prior,
            sub_prior,
            first_half,
            segment_capacity,
            roughness,
            seed
        )
        -- Fill in the second half
        fill_sumtable_for_distribution(
            table,
            start + first_half,
            prior + sub_prior,
            total - sub_prior,
            n_segments - first_half,
            segment_capacity,
            roughness,
            seed
        )
    end
end
anarchy.distribution.fill_sumtable_for_distribution = (
    fill_sumtable_for_distribution
)

--- Returns a sumtable for the same distribution, where entries are the
-- cumulative sum of all portions up to an index, rather than just the
-- portion at that index. Use functions like `sumtable_total`,
-- `sumtable_segments`, `sumtable_portion`, and `sumtable_segment` to get
-- information out of the sumtable efficiently.
--
-- @paramt table portions The array of per-segment item counts, indexed
--     starting at 1.
--
-- @returnt table The sumtable that encodes the same distribution (also
--     indexed from 1, like all normal Lua tables. Hahaha why would we
--     even mention this?).
local sumtable_from_portions = function(portions)
    sofar = 0
    result = {}
    for i, p in ipairs(portions) do
        sofar = sofar + p
        result[#result + 1] = sofar
    end

    return result
end
anarchy.distribution.sumtable_from_portions = sumtable_from_portions


--- Returns the total number of items across all segments in a sumtable,
-- which is just the last entry in that table.
--
-- @paramt table sumtable The table of sums describing the distribution
--     in question.
--
-- @returnt number The total number of items distributed.
local sumtable_total = function(sumtable)
    return sumtable[#sumtable]
end
anarchy.distribution.sumtable_total = sumtable_total


--- Returns the number of segments in the given sumtable, which is just
-- its length.
--
-- @paramt table sumtable The table of sums describing the distribution
--     in question.
--
-- @returnt number The number of segments in the distribution.
local sumtable_segments = function(sumtable)
    return #sumtable
end
anarchy.distribution.sumtable_segments = sumtable_segments


--- Returns the portion of the total items which is distributed into the
-- segment with the given index (starting from 1). Just needs to subtract
-- the entry to the left to figure this out. Raises an error if the index
-- is invalid.
--
-- @paramt table sumtable The table of sums describing the distribution
--     in question.
-- @paramt number i The segment to compute the portion for (from 1).
--
-- @returnt number The number of items distributed into the specified
--     segment.
local sumtable_portion = function(sumtable, i)
    -- First entry has nothing to subtract
    local here = sumtable[i]
    if here == nil then
        -- Out-of-bounds
        error(
            "Invalid index " .. i .. " for table with " .. #sumtable
         .. " entries."
        )
    end
    if i == 1 then
        return here
    else
        -- In-bounds entries just subtract sums to get value
        return here - sumtable[i - 1]
    end
end
anarchy.distribution.sumtable_portion = sumtable_portion


--- Returns a pair containing the index of the first item in the segment 
-- and then the number of items in that segment, like
-- `distribution_items`, but for a sumtable. Parameters:
--
-- @paramt table sumtable The table of sums describing the distribution
--     in question.
-- @paramt number i The segment to compute the portion for (from 1).
--
-- @return Two numbers: The index of the first item in the segment (from
--     1), then the number o items in the indicated segment.
local sumtable_items = function(sumtable, i)
    local portion = sumtable_portion(sumtable, i)  -- error here if bad i

    -- Read prior sum directly from adjacent table entry
    if i == 1 then
        prior = 0
    else
        prior = sumtable[i - 1]
    end

    return prior + 1, portion
end
anarchy.distribution.sumtable_items = sumtable_items


--- Uses binary search to find the index of the smallest sum in the given
-- sumtable that's greater than or equal to the given value breaking ties
-- towards earlier entries. Works in time proportional to the logarithm
-- of the sumtable size. This will be the segment which the given item
-- (out of the total) falls into.
--
-- Raises an error if the sumtable is empty, or if the item index is too
-- large for the sumtable or is zero or negative.
--
-- @paramt table sumtable The table of sums describing the distribution
--     in question.
-- @paramt number item The item (out of the total distributed, starting
--     from 0) that we want to find the segment for.
--
-- @returnt number The segment index (starting from 1) for the specified
--     item.
local sumtable_segment = function(sumtable, item)
    if #sumtable == 0 then
        error("Empty sumtable in sumtable_segment.")
    elseif item < 1 then
        error(
            "Zero or negative item index " .. item .. " not permitted in"
         .. " sumtable_segment."
        )
    elseif item > sumtable[#sumtable] then
        error(
            "Item index " .. item .. " is too large in sumtable_segment."
         .. " The sumtable only contains " .. sumtable[#sumtable]
         .. " items."
        )
    end

    local fr = 1  -- inclusive
    local to = #sumtable  -- inclusive
    local where = nil
    -- Keep splitting until we narrow it down to a single position (or none)
    while to - fr > 0 do
        where = fr + floor((to - fr) / 2)  -- halfway between
        if sumtable[where] < item then  -- definitely not here; it's farther
            fr = where + 1
        else  -- could be here
            to = where
        end
    end

    if to - fr < 0 then
        -- shouldn't be possible in a valid sumtable given the checks
        -- above...
        local frspec = "" .. fr .. " (value " .. sumtable[fr] .. ")"
        local tospec = "" .. to .. " (value " .. sumtable[to] .. ")"
        error(
            "Item index " .. item .. " not found in sumtable. Must be at"
         .. " least at index " .. frspec .. " but may not be past index "
         .. tospec .. "."
        )
    end

    assert(to == fr)
    -- Check the single remaining entry & return
    if sumtable[fr] < item then
        -- Shouldn't be possible with a valid sumtable given checks above
        -- that the index is in range of the total
        error(
            "Item index " .. item .. " not found in sumtable. Must be at"
         .. " index " .. fr .. " (value " .. sumtable[fr] .. ") but that"
         .. " value is too small."
        )
    else
        return fr
    end
end
anarchy.distribution.sumtable_segment = sumtable_segment


-- Return module table
return anarchy
