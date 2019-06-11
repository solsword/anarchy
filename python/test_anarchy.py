"""
test_anarchy.py
Testing module for anarchy reversible RNG library.
"""

import anarchy

VALUE_TESTS = {
  "posmod": [
    [ anarchy.posmod(-1, 7), 6 ],
    [ anarchy.posmod(0, 11), 0 ],
    [ anarchy.posmod(3, 11), 3 ],
    [ anarchy.posmod(-13, 11), 9 ],
    [ anarchy.posmod(15, 11), 4 ],
    [ anarchy.posmod(115, 10), 5 ],
    [ anarchy.posmod(115, 100), 15 ],
  ],
  "mask": [
    [ anarchy.mask(0), 0 ],
    [ anarchy.mask(1), 1 ],
    [ anarchy.mask(2), 3 ],
    [ anarchy.mask(4), 15 ],
    [ anarchy.mask(10), 1023 ],
  ],
  "byte_mask": [
    [ anarchy.byte_mask(0), 255 ],
    [ anarchy.byte_mask(1), 65280 ],
    [ anarchy.byte_mask(2), 16711680 ],
  ],
  "swirl": [
    [ anarchy.swirl(2, 1), 1 ],
    [ anarchy.swirl(4, 1), 2 ],
    [ anarchy.swirl(8, 2), 2 ],
    [ anarchy.swirl(1, 1), 0x8000000000000000 ],
    [ anarchy.swirl(2, 2), 0x8000000000000000 ],
    [ anarchy.swirl(1, 2), 0x4000000000000000 ],
    [ anarchy.rev_swirl(1, 2), 4 ],
    [ anarchy.rev_swirl(1, 3), 8 ],
    [ anarchy.rev_swirl(2, 2), 8 ],
    [ anarchy.rev_swirl(0x8000000000000000, 1), 1 ],
    [ anarchy.rev_swirl(0x8000000000000000, 2), 2 ],
    [ anarchy.rev_swirl(0x8000000000000000, 3), 4 ],
    [ anarchy.rev_swirl(0x4000000000000000, 3), 2 ],
    [ anarchy.rev_swirl(0x0000000000101030, 1), 0x0000000000202060 ],
    [
      anarchy.rev_swirl(anarchy.swirl(1098301, 17), 17),
      1098301
    ],
  ],
  "fold": [
    # TODO: Verify these!
    [ anarchy.fold(22908, 7), 50375224738208124 ],
    [ anarchy.fold(18201, 18), 1280781512777680665 ],
    [ anarchy.fold(anarchy.fold(18201, 18), 18), 18201 ],
    [ anarchy.fold(anarchy.fold(89348393, 1289), 1289), 89348393 ],
  ],
  "flop": [
    # TODO: Verify these!
    [ anarchy.flop(0xf0f0f0f0), 0x0f0f0f0f ],
    [ anarchy.flop(0x0f0f0f0f), 0xf0f0f0f0 ],
    [ anarchy.flop(22908), 38343 ],
    [ anarchy.flop(18201), 29841 ],
    [ anarchy.flop(anarchy.flop(3892389)), 3892389 ],
    [ anarchy.flop(anarchy.flop(489248448)), 489248448 ],
  ],
  "scramble": [
    [
      anarchy.scramble(
        anarchy.rev_swirl(0x03040610 | 0x40004001, 1)
      ),
      0x40004001
    ],
    [
      anarchy.rev_scramble(0x40004001),
      anarchy.rev_swirl(0x03040610 | 0x40004001, 1)
    ],
    [ anarchy.rev_scramble(anarchy.scramble(17)), 17 ],
    [ anarchy.rev_scramble(anarchy.scramble(8493489)), 8493489 ],
    [ anarchy.scramble(anarchy.rev_scramble(8493489)), 8493489 ],
  ],
  "prng": [
    # TODO: Verify these!
    [ anarchy.prng(489348, 373891), 18107188676709054266 ],
    [ anarchy.rev_prng(1766311112, 373891), 14566213110237565854 ],
    [ anarchy.prng(0, 0), 15132939213242511212 ],
    [ anarchy.rev_prng(15132939213242511212, 0), 0 ],
    [ anarchy.prng(anarchy.rev_prng(1782, 39823), 39823), 1782 ],
    [ anarchy.rev_prng(anarchy.prng(1782, 39823), 39823), 1782 ],
  ],
  "lfsr": [
    # TODO: Verify these!
    [ anarchy.lfsr(489348), 244674 ],
    [ anarchy.lfsr(1766932808), 883466404 ],
  ],
  "udist": [
    # TODO: Verify and/or fix these!
    [ anarchy.udist(0), 0.39901845521416135 ],
    [ anarchy.udist(8329801), 0.9082340390686318 ],
    [ anarchy.udist(58923), 0.11871384160294333 ],
  ],
  "idist": [
    # TODO: Verify and/or fix these!
    [ anarchy.idist(0, 0, 1), 0 ],
    [ anarchy.idist(0, 3, 25), 11 ],
    [ anarchy.idist(58923, 3, 25), 5 ],
    [ anarchy.idist(58923, -2, -4), -3 ],
  ],
  "expdist": [
    # TODO: Verify and/or fix these!
    [ anarchy.expdist(0), 1.018382104858997 ],
    [ anarchy.expdist(8329801), 4.77702769214421 ],
    [ anarchy.expdist(58923), 0.2527457897844567 ],
  ],
  "cohorts": [
    [ anarchy.cohort(17, 3), 5 ],
    [ anarchy.cohort(10, 10), 1 ],
    [ anarchy.cohort(9, 10), 0 ],
    [ anarchy.cohort_inner(17, 3), 2 ],
    [ anarchy.cohort_inner(10, 10), 0 ],
    [ anarchy.cohort_inner(9, 10), 9 ],
    [ anarchy.cohort_and_inner(9, 10)[0], 0 ],
    [ anarchy.cohort_and_inner(9, 10)[1], 9 ],
    [ anarchy.cohort_outer(0, 3, 112), 3 ],
    [ anarchy.cohort_outer(1, 3, 112), 115 ],
    [ anarchy.cohort_outer(-1, 3, 112), 18446744073709551507 ],
  ],
  "cohort_interleave": [
    [ anarchy.cohort_interleave(3, 12), 6 ],
    [ anarchy.cohort_interleave(7, 12), 9 ],
  ],
  "cohort_spin": [
    [ anarchy.cohort_spin(0, 2, 1048239), 1 ],
    [ anarchy.rev_cohort_spin(1, 2, 1048239), 0 ],
  ],
  "cohort_mix": [
    [ anarchy.cohort_mix(0, 3, 0), 2 ],
    [ anarchy.cohort_mix(1, 3, 0), 1 ],
    [ anarchy.cohort_mix(2, 3, 0), 0 ],
    [ anarchy.rev_cohort_mix(2, 3, 0), 0 ],
    [ anarchy.rev_cohort_mix(1, 3, 0), 1 ],
    [ anarchy.rev_cohort_mix(0, 3, 0), 2 ],
  ],
  "cohort_fold": [
    [ anarchy.cohort_fold(0, 3, 0), 0 ],
    [ anarchy.cohort_fold(1, 3, 0), 2 ],
    [ anarchy.cohort_fold(2, 3, 0), 1 ],
    [ anarchy.rev_cohort_fold(0, 3, 0), 0 ],
    [ anarchy.rev_cohort_fold(1, 3, 0), 2 ],
    [ anarchy.rev_cohort_fold(2, 3, 0), 1 ],
  ],
  "cohort_shuffle": [
    [ anarchy.cohort_shuffle(0, 3, 17), 0 ],
    [ anarchy.cohort_shuffle(1, 3, 17), 2 ],
    [ anarchy.cohort_shuffle(2, 3, 17), 1 ],
    [ anarchy.rev_cohort_shuffle(0, 3, 17), 0 ],
    [ anarchy.rev_cohort_shuffle(2, 3, 17), 1 ],
    [ anarchy.rev_cohort_shuffle(1, 3, 17), 2 ],
  ],
}

TEST_VALUES = [
  0,
  1,
  3,
  17,
  48,
  64,
  1029,
  8510938,
  1928301928,
  1 << 31
]

EXEC_TESTS = {}

def test(fn):
  global EXEC_TESTS
  EXEC_TESTS[fn.__name__] = fn

@test
def unit_ops():
  result = 0
  messages = []
  seed = 129873918231
  for o in (0, 12, 24, 36, 48):
    print("\nOffset:", o)
    for i in range(1 << 12):
      if i % (1 << 8) == (1 << 8) - 1:
        print(i/(1 << 12), end='\r')
      x = (i << o)
      if (x != anarchy.flop(anarchy.flop(x))):
        messages.append("flop @ {}".format(x))
        result += 1
      if (x != anarchy.rev_scramble(anarchy.scramble(x))):
        messages.append("scramble @ {}".format(x))
        result += 1
      if (x != anarchy.prng(anarchy.rev_prng(x, seed), seed)):
        messages.append("prng @ {}".format(x))
        result += 1
      r = x
      p = x
      for j, y in enumerate(TEST_VALUES):
        if (
          x != anarchy.rev_swirl(
            anarchy.swirl(x, y),
            y
          )
        ):
          messages.append("swirl @ {}, {}".format(x, y))
          result += 1

        if (x != anarchy.fold(anarchy.fold(x, y), y)):
          messages.append("fold @ {}, {}".format(x, y))
          result += 1

        if (r != anarchy.scramble(anarchy.rev_scramble(r))):
          messages.append("chained scramble @ {}".format(r))
          result += 1

        r = anarchy.rev_scramble(r)
        if (p != anarchy.rev_prng(anarchy.prng(p, seed), seed)):
          messages.append("chained prng @ {}".format(p))
          result += 1

        p = anarchy.prng(p, seed)

  if result != 0:
    print("unit_ops failures:")
    for m in messages:
      print(m)

  return result

@test
def cohort_ops():
  result = 0
  messages = []
  cohort_sizes = [ 3, 12, 17, 32, 1024 ]
  for idx in range(len(cohort_sizes)):
    cs = cohort_sizes[idx]
    observed = {
      "interleave": {},
      "fold": {},
      "spin": {},
      "flop": {},
      "mix": {},
      "spread": {},
      "upend": {},
      "shuffle": {},
    }
    for i in range(cs):
      # interleave
      x = anarchy.cohort_interleave(i, cs)
      rc = anarchy.rev_cohort_interleave(x, cs)
      if (i != rc):
        messages.append("interleave({}, {})→{}→{}".format( i, cs, x, rc))
        result += 1

      v = str(x)
      if v in observed["interleave"]:
        observed["interleave"][v] += 1
      else:
        observed["interleave"][v] = 1

    for j, seed in enumerate(TEST_VALUES):
      observed["fold"][str(j)] = {}
      observed["spin"][str(j)] = {}
      observed["flop"][str(j)] = {}
      observed["mix"][str(j)] = {}
      observed["spread"][str(j)] = {}
      observed["upend"][str(j)] = {}
      observed["shuffle"][str(j)] = {}
      for i in range(cs):
        # fold
        x = anarchy.cohort_fold(i, cs, seed)
        rc = anarchy.rev_cohort_fold(x, cs, seed)
        if i != rc:
          messages.append(
            "fold({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["fold"][str(j)]:
          observed["fold"][str(j)][v] += 1
        else:
          observed["fold"][str(j)][v] = 1

        # spin
        x = anarchy.cohort_spin(i, cs, seed)
        rc = anarchy.rev_cohort_spin(x, cs, seed)
        if i != rc:
          messages.append(
            "spin({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["spin"][str(j)]:
          observed["spin"][str(j)][v] += 1
        else:
          observed["spin"][str(j)][v] = 1

        # flop
        x = anarchy.cohort_flop(i, cs, seed)
        rc = anarchy.cohort_flop(x, cs, seed)
        if i != rc:
          messages.append("flop({}, {}, {})→{}→{}".format(i, cs, seed, x, rc))
          result += 1

        v = str(x)
        if v in observed["flop"][str(j)]:
          observed["flop"][str(j)][v] += 1
        else:
          observed["flop"][str(j)][v] = 1

        # mix
        x = anarchy.cohort_mix(i, cs, seed)
        rc = anarchy.rev_cohort_mix(x, cs, seed)
        if i != rc:
          messages.append(
            "mix({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["mix"][str(j)]:
          observed["mix"][str(j)][v] += 1
        else:
          observed["mix"][str(j)][v] = 1

        # spread
        x = anarchy.cohort_spread(i, cs, seed)
        rc = anarchy.rev_cohort_spread(x, cs, seed)
        if i != rc:
          messages.append(
            "spread({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["spread"][str(j)]:
          observed["spread"][str(j)][v] += 1
        else:
          observed["spread"][str(j)][v] = 1

        # upend
        x = anarchy.cohort_upend(i, cs, seed)
        rc = anarchy.cohort_upend(x, cs, seed)
        if i != rc:
          messages.append(
            "upend({}, {}, {})→{}→{}".format(i, cs, i, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["upend"][str(j)]:
          observed["upend"][str(j)][v] += 1
        else:
          observed["upend"][str(j)][v] = 1

        # shuffle
        x = anarchy.cohort_shuffle(i, cs, seed)
        rc = anarchy.rev_cohort_shuffle(x, cs, seed)
        if i != rc:
          messages.append(
            "shuffle({}, {}, {})→{}→{}".format(i, cs, seed, x, rc)
          )
          result += 1

        v = str(x)
        if v in observed["shuffle"][str(j)]:
          observed["shuffle"][str(j)][v] += 1
        else:
          observed["shuffle"][str(j)][v] = 1


    for prp in observed:
      if prp == "interleave":
        for i in range(cs):
          v = str(i)
          if v in observed[prp]:
            count = observed[prp][v]
          else:
            count = 0

          if count != 1:
            messages.append(
              "{}({}, {}) found {}".format(prp, i, cs, count)
            )
            result += 1

      else:
        for j, seed in enumerate(TEST_VALUES):
          k = str(j)
          for i in range(cs):
            v = str(i)
            count = observed[prp][k].get(v, 0)
            if count != 1:
              messages.append(
                "{}({}, {}, {}) found {}".format(prp, i, cs, seed, count)
              )
              result += 1


  if result != 0:
    print("cohort_ops failures:")
    for m in messages:
      print('  ' + m)

  return result

# TODO: Distribution function tests!

def run_value_tests():
  print("Starting value tests...")
  for t in VALUE_TESTS:
    test_count = len(VALUE_TESTS[t])
    passed = 0
    for index, sub_t in enumerate(VALUE_TESTS[t]):
      if sub_t[0] == sub_t[1]:
        passed += 1
      else:
        print("Test failed: {}.{}".format(t, index))
        print("  expected: {} got: {}".format(sub_t[1], sub_t[0]))

    print("Suite '{}': passed {} / {}".format(t, passed, test_count))

  print("Done with value tests.")

def run_exec_tests():
  for t in EXEC_TESTS:
    result = EXEC_TESTS[t]()
    if result != 0:
      print("Test '{}' failed {} sub-tests.".format(t, result))
    else:
      print("Test '{}' succeeded.".format(t))

# do it!
if __name__ == "__main__":
  run_value_tests()
  run_exec_tests()
