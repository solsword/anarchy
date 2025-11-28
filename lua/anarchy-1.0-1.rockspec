package = 'Anarchy'
version = '1.0-2'
source = {
    url = "https://cs.wellesley.edu/~pmwh/anarchy/lua/anarchy.lua"
}
description = {
    summary = "A module for incremental & reversible chaos.",
    detailed = [[
Anarchy is a module for incremental and reversible chaos.

This is the 64-bit version compatible with Lua 5.3+. If you want to use
it with Lua 5.1, 5.2 or LuaJIT, you'll need to grab the `anarchy32`
library instead.

Capabilities include:

- Reversible pseudo-random number generator
- Convenience functions for pseudo-normal, exponential, etc. distributions
- Reversible & incremental shuffling
- Reversible & incremental distribution of items among segments

Also available in Python, Javascript, C, and C# (with a Unity demo).
]],
    homepage = "https://cs.wellesley.edu/~pmwh/anarchy/lua/",
    license = "BSD 3-clause"
}
dependencies = {
    "lua >= 5.3",
}
build = {
    type = "builtin",
    modules = {
        anarchy = "anarchy.lua"
    }
}
