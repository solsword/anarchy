# anarchy

Incremental & reversible chaos library. Generates pseudo-random numbers
(not suitable for cryptographic or statistical applications) in a
reversible and incremental manner.

"Reversible" means that you can take a random result value and use
reverse random functions to determine the seed value it came from, or
take a shuffled item position and determine which original position it
had. This property is useful for implementing things like a compass or
'rare item radar' that can lead the way to the nearest
randomly-distributed item of a particular type, since you can not only
ask "I'm generating the tile at 3, 17, what goes here?" but also "Which
tile was treasure #25 assigned to?"

"Incremental" means that you can generate the index of one shuffled
item without generating and storing an array containing the total
ordering of the items (or figure out which segment an item got
distributed into without storing the full mapping of items to
segments). This property is useful in situations like Minecraft, where
we need to generate chunks of a larger world based on the order the
player explores them in. Pre-generating entire regions (or even data
structures to support placement of items in those regions) is not
feasible, and instead with `anarchy`, you can simply request the
portions of a shuffle or distribution that you need as you need them.
In general this also means that you don't need to manage and hand
around an "RNG" instance in the code, but can instead share a single
integer seed and have each part of the code derive all the information
it needs from that alone.

Capabilities include:

- Reversible pseudo-random number generator
- Convenience functions for pseudo-normal, exponential, etc. distributions
- Reversible & incremental shuffling
- Reversible & incremental random distribution of items among segments

The core API is available in C, Python, Javascript, and Unity/C#, and
includes:

- Reversible pseudo-random number generation:
    * `prng` and `rev_prng` the forward and reverse PRNG functions
    * `lfsr` as a simple non-reversible PRNG (Linear Feedback Shift
        Register)
- Distribution functions that turn PRNG outputs into more useful numbers
    (only in non-C versions for now; Javascript versions have different
    names):
    * `uniform` for a uniform floating-point value between 0 and 1.
    * `normalish` for a pseudo-gaussian value between 0 and 1.
    * `integer` for an integer between given min (inclusive) and max
        (exclusive).
    * `exponential`, and `truncated_exponential` for floating-point
        exponential and truncated exponential distributions
- Incremental & reversible shuffling:
    * `cohort_shuffle` and `rev_cohort_shuffle` for finding
        shuffled/unshuffled indices one-at-a-time.
- Incremental & reversible distribution:
    * `distribution_portion` - For *N* items distributed among *K*
        segments each of maximum size *S*, how many items are in
        segment *k*? 
    * `distribution_prior_sum` - For *N* items distributed among *K*
        segments of maximum size *S*, how many of the items are in
        segments that preceed segment *k*?
    * `distribution_items` - Convenience function that returns both the
        prior sum and portion for a segment without repeating work.
    * `distribution_segment` - For *N* items distributed among *K*
        segments of maximum size *S*, which segment does item *n* get
        distributed to?


## Core documentation

Reading the source code comments for the implementation you're using is
the best way to get details on specific functions. Online you can find:

- [Documentation for the Python
    version](https://cs.wellesley.edu/~pmwh/anarchy/python/doc.html) -
    This is the most complete online documentation available. Of course
    each implementation has its own nuances that may be slightly
    different from the Python version.
- [Documentation for the main Lua
    version](https://cs.wellesley.edu/~pmwh/anarchy/lua/docs/index.html)
    This documentation applies to the Lua 5.3+ version. See also [the
    Lua version README
    page](https://cs.wellesley.edu/~pmwh/anarchy/lua/).


Each individual implementation's source code is documented.


## Example code

Example code using the JavaScript implementation that demonstrates things
to do with the library. These examples run live in your browser and
include code snippets showing how things are accomplished; reading
through these is the fastest way to get a sense of how to use the
library and what it's good for.

[Examples](https://cs.wellesley.edu/~pmwh/anarchy/js/examples.html)


## Unity Example

The `unity/anarchy` folder is a complete Unity project which includes a
component implementation of the library and a simple demo component
that uses it to shuffle tiles in a `Tilemap`. When it's running, you
can left-click to advance to the next seed, or right-click to go back
to the previous seed. The code in the `Tilepicker` component
demonstrates how to use `anarchy`.

That said, I'm not a Unity expert and this probably isn't the right way
to release the library; it seems like a plugin would be more
appropriate, but I haven't yet figured out the details of building and
deploying one. If you'd like to use anarchy more seriously in a Unity
context, let me know (even just by opening an issue) and I can make
that more of a priority.
