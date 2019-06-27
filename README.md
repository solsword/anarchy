# anarchy

Incremental & reversible chaos library. Generates pseudo-random numbers (not
suitable for cryptographic or statistical applications) in a reversible and
incremental manner.

Capabilities include:

- Reversible PRNG
- Reversible & incremental shuffling
- Reversible & incremental random distribution (items among ranges)

The core API is available in C, Python, and Javascript, and includes:

- Reversible pseudo-random number generation:
    * `prng` and `rev_prng` the forward and reverse PRNG functions
    * `lfsr` as a simple non-reversible PRNG (Linear Feedback Shift Register)
    * `udist`, `idist`, and `expdist` for floating-point, integer-range, and
      floating-point exponential distributions (only in Python and Javascript
      for now).
- Incremental & reversible shuffling:
    * `cohort_shuffle` and `rev_cohort_shuffle` for finding shuffled/unshuffled
      indices one-at-a-time.
- Incremental & reversible distribution:
    * `distribution_portion` - For *N* items distributed among *K* segments
      each of maximum size *S*, how many items are in segment *k*? 
    * `distribution_prior_sum` - For *N* items distributed among *K* segments
      of maximum size *S*, how many of the items are in segments that preceed
      segment *k*?
    * `distribution_segment` - For *N* items distributed among *K* segments of
      maximum size *S*, which segment does item *n* get distributed to?

## Further work

- One long term goal is to use these operations to support family tree
  generation.
