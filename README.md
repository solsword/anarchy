# anarchy

Incremental & reversible chaos library. Generates pseudo-random numbers (not
suitable for cryptographic or statistical applications) in a reversible and
incremental manner.

Capabilities include:

- Reversible PRNG
- Reversible & incremental shuffling
- Reversible & incremental random distribution (items among ranges)

The core API is available in C, Python, Javascript, and Unity/C#, and includes:

- Reversible pseudo-random number generation:
    * `prng` and `rev_prng` the forward and reverse PRNG functions
    * `lfsr` as a simple non-reversible PRNG (Linear Feedback Shift Register)
- Distribution functions that turn PRNG outputs into more useful numbers
  (only in Python and Javascript for now):
    * `udist` for a uniform floating-point value between 0 and 1.
    * `pgdist` for a pseudo-gaussian value between 0 and 1.
    * `idist` for an integer between given min and max values.
    * `expdist`, and `trexpdist` for floating-point exponential and
      truncated exponential distributions
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

## Core documentation

Full documentation for all core functions pulled directly from the Python
implementation (applies to all implementations):

[Core Documentation](https://solsword.github.io/anarchy/python/doc.html)

## Example code

Example code using the JavaScript implementation that demonstrates things
to do with the library.

[Examples](https://solsword.github.io/anarchy/js/examples.html)

## Unity Example

The `unity/anarchy` folder is a complete Unity project which includes a
component implementation of the library and a simple demo component that uses
it to shuffle tiles in a `Tilemap`. When it's running, you can left-click to
advance to the next seed, or right-click to go back to the previous seed. The
code in the `Tilepicker` component demonstrates how to use `anarchy`.

That said, I'm not a Unity expert and this probably isn't the right way to
release the library; it seems like a plugin would be more appropriate, but I
haven't yet figured out the details of building and deploying one. If you'd
like to use anarchy more seriously in a Unity context, let me know (even just
by opening an issue) and I can make that more of a priority.
