#!/usr/bin/env python3
"""
Prime number helper.

This is trash. Probably worth the memory to sieve up from 0 since we
spend so much time checking modularity against non-primes we wouldn't
need to otherwise. Values are available online anyways...
"""

import sys

bits = 64
if len(sys.argv) > 1:
    try:
        bits = int(sys.argv[1])
    except ValueError:
        print("Invalid bit depth {sys.argv[1]}.")
else:
    bits = 64

print(f"Finding a prime near {bits} bits...")

start = (1 << bits) - 1

if len(sys.argv) > 2:
    try:
        start = int(sys.argv[2])
    except ValueError:
        print(f"Invalid start point {sys.argv[2]}")

considered = 0
for n in range(start, 0, -2):
    # Update limit only every 10 iterations; unlikely to reach it in
    # most cases...
    if considered % 10 == 0:
        lim = int(n ** 0.5) + 1
    print(f"Checking {n}...")
    # Check all possible divisors (plus maybe some extras)
    prime = True
    checked = 0
    for div in range(3, lim, 2):
        if n % div == 0:
            prime = False
            break
        checked += 1
        print(f"{div}/{lim}    ", end="\r")
    if prime:
        print(f"\nFound prime {n} after {considered + 1} tries.")
    considered += 1
