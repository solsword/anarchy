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

This is the directory for the Lua versions; the library is also
available in C, Unity/C#, Python, and Javascript.

There are four versions of the library available in Lua:

1. For Lua 5.1, 5.2 and LuaJIT, [`anarchy32.lua`](anarchy32.lua). If
   you need a version that defines the `anarchy` table as a global
   variable rather than returning it module-style, get
   [`anarchy32_global.lua`](anarchy32_global.lua) instead.
2. For Lua 5.3+, using 64-bit integers where available and the new
   built-in bitwise operators that should make things faster, get
   [`anarchy.lua`](anarchy.lua). The corresponding global-variable
   version is [`anarchy_global.lua`](anarchy.lua).

The module versions are also on
[Luarocks](https://luarocks.org/modules/solsword/anarchy) so you can do
`luarocks install anarchy` or `luarocks install anarchy32` to install
them if you want.

## Links

The [main README page](https://cs.wellesley.edu/~pmwh/anarchy/) has
links to live examples and documentation for some other versions.

[Documentation for the `anarchy32`
version](https://cs.wellesley.edu/~pmwh/anarchy/lua/docs32/index.html)
and [documentation for the `anarchy`
version](https://cs.wellesley.edu/~pmwh/anarchy/lua/docs/index.html)
are available.
