package = 'Anarchy'
version = '1.0'
source = {
    url = "..."
}
description = {
    summary = "A module for incremental & reversible chaos.",
    detailed = [[
Anarchy is a module for incremental and reversible chaos.

Capabilities include:

- Reversible pseudo-random number generator
- Convenience functions for pseudo-normal, exponential, etc. distributions
- Reversible & incremental shuffling
- Reversible & incremental distribution of items among segments

Also available in Python, Javascript, C, and C# (with a Unity demo).
]],
    homepage = "https://cs.wellesley.edu/~pmwh/anarchy/",
    license = "BSD 3-clause"
}
dependencies = {
    "lua >= 5.1"
}
build = {
}
