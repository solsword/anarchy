import sys
import re

import anarchy


def unindent(docstr):
    """
    Strips indentation from a docstring that's in this format.
    """
    indent = 0
    for i, c in enumerate(docstr):
        if i == 0 and c == '\n':
            continue
        if c != ' ':
            break
        indent += 1
    lines = docstr.split('\n')
    result = ''
    for line in lines:
        if line == '':
            result += '\n'
        else:
            if line[:indent].strip() != '':
                print(
                    "Warning: non-indented line:\n{}".format(line),
                    file=sys.stderr
                )
            result += line[indent:] + '\n'

    return result


PRNG_FUNCTIONS = [
  ("rng.prng", anarchy.rng.prng),
  ("rng.rev_prng", anarchy.rng.rev_prng),
  ("rng.uniform", anarchy.rng.uniform),
  ("rng.normalish", anarchy.rng.normalish),
  ("rng.flip", anarchy.rng.flip),
  ("rng.integer", anarchy.rng.integer),
  ("rng.exponential", anarchy.rng.exponential),
  ("rng.truncated_exponential", anarchy.rng.truncated_exponential),
]

COHORT_FUNCTIONS = [
  ("cohort.cohort", anarchy.cohort.cohort),
  ("cohort.cohort_inner", anarchy.cohort.cohort_inner),
  ("cohort.cohort_outer", anarchy.cohort.cohort_outer),
  ("cohort.cohort_shuffle", anarchy.cohort.cohort_shuffle),
  ("cohort.rev_cohort_shuffle", anarchy.cohort.rev_cohort_shuffle),
  ("cohort.distribution_portion", anarchy.cohort.distribution_portion),
  ("cohort.distribution_prior_sum", anarchy.cohort.distribution_prior_sum),
  ("cohort.distribution_segment", anarchy.cohort.distribution_segment),
]


def as_link(identifier):
    """
    Returns Markdown link syntax for a link to the given identifier, or
    just the identifier itself in backticks if it's not a thing we can
    link to.
    """
    matches = [
        fn
        for (fn, f) in PRNG_FUNCTIONS + COHORT_FUNCTIONS
        if identifier == fn
    ]
    link_to = None
    if len(matches) == 1:
        link_to = matches[0]
    else: # not something we can link to
        rmatches = [
            fn for (fn, f) in PRNG_FUNCTIONS + COHORT_FUNCTIONS
            if fn.endswith("." + identifier)
        ]
        if len(rmatches) == 1:
            link_to = rmatches[0]

    if link_to is None:
        return f"`{identifier}`"
    else:
        return f"[`{identifier}`](#{link_to})"


def make_links(doc):
    """
    Turns back-tick-quoted names from the functions we know about into
    links to the doc entry for those functions.
    """
    return re.sub(
        r"(?<!### )`([a-zA-Z_.]+)`",
        lambda match: as_link(match.group(1)),
        doc
    )


doc = """\
---
title: Anarchy Library Documentation
author: Peter Mawhorter (pmawhorter@gmail.com)
...
"""
with open("README.md", 'r') as fin:
    doc += fin.read()
doc += """

## Contents

### Single Number Functions

Deal with individual random numbers.

"""
for fullname, f in PRNG_FUNCTIONS:
    doc += f"- [`{fullname}`](#{fullname})\n"

doc += """

### Cohort Functions

Deal consistently with numbers in groups.

"""
for fullname, f in COHORT_FUNCTIONS:
    doc += f"- [`{fullname}`](#{fullname})\n"

doc += """
## Functions

"""

for fullname, f in PRNG_FUNCTIONS:
    doc += f"\n### `{fullname}`\n\n"
    doc += unindent(make_links(f.__doc__)) + '\n'
for fullname, f in COHORT_FUNCTIONS:
    doc += f"\n### `{fullname}`\n\n"
    doc += unindent(make_links(f.__doc__)) + '\n'


with open("doc.md", 'w') as fout:
    fout.write(doc)
