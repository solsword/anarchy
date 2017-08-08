#!/usr/bin/env python3

import math

def closed_form(n, g):
  return g * n * (n + 1) // 2

def full_sum(x, g):
  s = 0
  n = 0
  for i in range(x):
    n += g
    s += n
  return s

def inverse_sum(s, g):
  return int(math.floor((0.25 + 2*s//g)**0.5 - 0.5))

for i in range(10):
  print(
    "{:6d}  {:<6d} | {:6d}  {:<6d} | {:6d}  {:<6d} | {:6d}  {:<6d}".format(
      closed_form(i, 1), full_sum(i, 1),
      closed_form(i, 2), full_sum(i, 2),
      closed_form(i, 3), full_sum(i, 3),
      closed_form(i, 4), full_sum(i, 4)
    )
  )

for i in (
  list(range(1000))
+ list(range(1000, 1000000, 8017))
):
  print(i, end="\r")
  for g in range(1,8,2):
    if closed_form(i, g) != full_sum(i, g):
      print(
        "closed_form failed for x={}, g={} ({} vs. {})".format(
          i, g, closed_form(i, g), full_sum(i, g)
        )
      )

print("\n")

for i in range(10):
  print(
    "{:6d} ? {:6d}  {:<6d} | {:6d} ? {:6d}  {:<6d} | {:6d} ? {:6d}  {:<6d}"
    .format(
      closed_form(i, 1), i, inverse_sum(closed_form(i,1),1),
      closed_form(i, 2), i, inverse_sum(closed_form(i,2),2),
      closed_form(i, 3), i, inverse_sum(closed_form(i,3),3)
    )
  )

for i in (
  list(range(1000000))
+ list(range(1000000, 1000000000, 8001004))
):
  print(i, end="\r")
  for g in range(1,8,2):
    cf = closed_form(i, g)
    inv = inverse_sum(cf, g)
    if inv != i:
      print("inverse failed for s={}, g={} ({} vs. {})".format(cf, g, inv, i))

print()
