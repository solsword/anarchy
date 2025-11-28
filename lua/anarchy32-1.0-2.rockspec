package = 'Anarchy32'
version = '1.0-2'
source = {
    url = "https://cs.wellesley.edu/~pmwh/anarchy/lua/anarchy32.lua"
}
description = {
    summary = "A module for incremental & reversible chaos.",
    detailed = [[
Anarchy32 is a module for incremental and reversible chaos.

This is the 32-bit version compatible with Lua 5.1, 5.2, and LuaJIT. Use
the 'anarchy' module instead for compatibility with Lua 5.3+, which is
also faster and has better quality since it can use 64-bit integers
instead of 32-bit 'integers' stored as floats.

Capabilities include:

- Reversible pseudo-random number generator
- Convenience functions for pseudo-normal, exponential, etc. distributions
- Reversible & incremental shuffling
- Reversible & incremental distribution of items among segments

Also available in Python, Javascript, C, and C# (with a Unity demo).

For use with Lua 5.1, you'll need to install the 'bit32' library that
backports the built-in bit32 module from Lua 5.2.
]],
    homepage = "https://cs.wellesley.edu/~pmwh/anarchy/lua/",
    license = "BSD 3-clause"
}
dependencies = {
    "lua >= 5.1, <= 5.2",
}
build = {
    type = "builtin",
    modules = {
        anarchy32 = "anarchy32.lua"
    }
}
