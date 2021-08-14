"""
Visual demonstrations of anarchy system.

viz.py
"""

import random
import os

import PIL.Image

from . import rng, cohort

MAX_RANDOM = rng.ID_MASK


def builtin_next():
    """
    Returns random numbers using built-in random.
    """
    return random.randint(0, MAX_RANDOM)


def demo_prng():
    """
    Creates images that demonstrate the PRNG functionality and compare
    it against the built-in irreversible PRNG.
    """
    seed = 2318230981
    random.seed(seed)
    prev_a = 472938
    prev_b = builtin_next()
    a_seq = PIL.Image.new("RGB", (100, 100))
    b_seq = PIL.Image.new("RGB", (100, 100))
    a_map = {}
    a_map_max = 0
    b_map = {}
    b_map_max = 0
    for x in range(100):
        for y in range(100):
            next_a = rng.prng(prev_a, seed)
            c_x = round(prev_a / MAX_RANDOM * 99)
            c_y = round(next_a / MAX_RANDOM * 99)
            if c_x not in a_map:
                a_map[c_x] = {}
            if c_y not in a_map[c_x]:
                a_map[c_x][c_y] = 0
            else:
                a_map[c_x][c_y] += 1
                if a_map[c_x][c_y] > a_map_max:
                    a_map_max = a_map[c_x][c_y]
            prev_a = next_a
            brightness = round(next_a / MAX_RANDOM * 255)
            a_seq.putpixel((x, y), (brightness,) * 3)

            next_b = builtin_next()
            c_x = round(prev_b / MAX_RANDOM * 99)
            c_y = round(next_b / MAX_RANDOM * 99)
            if (c_x < 0 or c_x >= 100 or c_y < 0 or c_y >= 100):
                print(prev_b, next_b, c_x, c_y)
            if c_x not in b_map:
                b_map[c_x] = {}
            if c_y not in b_map[c_x]:
                b_map[c_x][c_y] = 0
            else:
                b_map[c_x][c_y] += 1
                if b_map[c_x][c_y] > b_map_max:
                    b_map_max = b_map[c_x][c_y]
            prev_b = next_b
            brightness = round(next_b / MAX_RANDOM * 255)
            b_seq.putpixel((x, y), (brightness,) * 3)

    a_coords = PIL.Image.new("RGB", (100, 100))
    b_coords = PIL.Image.new("RGB", (100, 100))
    for kx in a_map:
        for ky in a_map[kx]:
            val = round(a_map[kx][ky] / a_map_max * 255)
            a_coords.putpixel((kx, ky), (val,) * 3)

    for kx in b_map:
        for ky in b_map[kx]:
            val = round(b_map[kx][ky] / b_map_max * 255)
            b_coords.putpixel((kx, ky), (val,) * 3)

    a_seq.save(os.path.join("demos", "rng_seq_anarchy.png"), format="png")
    b_seq.save(os.path.join("demos", "rng_seq_builtin.png"), format="png")
    a_coords.save(
        os.path.join("demos", "rng_coords_anarchy.png"),
        format="png"
    )
    b_coords.save(
        os.path.join("demos", "rng_coords_builtin.png"),
        format="png"
    )


def demo_shuffle():
    """
    Creates images that demonstrate the shuffle functionality and
    compare it against irreversible shuffling using the built-in random
    module.
    """
    seed = 8231093281
    random.seed(seed)

    a_demo = PIL.Image.new("RGB", (100, 100))
    b_demo = PIL.Image.new("RGB", (100, 100))

    # Twenty rows of repeated shuffles of a single initial list
    seq_a = list(range(100))
    seq_b = list(range(100))
    for i in range(20):
        y = i
        for x in range(100):
            a_demo.putpixel((x, y), (round(seq_a[x] / 100 * 256),) * 3)
            b_demo.putpixel((x, y), (round(seq_b[x] / 100 * 256),) * 3)

        seq_a_new = [
            seq_a[cohort.cohort_shuffle(j, 100, seed)]
                for j in range(len(seq_a))
        ]
        seq_a = seq_a_new

        random.shuffle(seq_b)

    # Twenty rows of different shuffles of the same initial list
    seq_a = list(range(100))
    seq_b = list(range(100))
    seq_a_here = seq_a
    seq_b_here = seq_b
    for i in range(20):
        y = i + 25
        for x in range(100):
            a_demo.putpixel((x, y), (round(seq_a_here[x] / 100 * 256),) * 3)
            b_demo.putpixel((x, y), (round(seq_b_here[x] / 100 * 256),) * 3)

        seq_a_here = [
            seq_a[cohort.cohort_shuffle(j, 100, seed + i)]
                for j in range(len(seq_a))
        ]

        seq_b_here = seq_b[:]
        random.shuffle(seq_b_here)

    # How many trials to run
    N_TRIALS = 1000

    # 50x50 square of x vs y shuffle input/output positions
    seq_a = list(range(50))
    seq_b = list(range(50))

    a_map = {}
    a_map_max = 0
    b_map = {}
    b_map_max = 0
    for i in range(N_TRIALS):
        shuf_a = [
            seq_a[cohort.cohort_shuffle(j, 50, seed + i)]
                for j in range(len(seq_a))
        ]
        for j, n in enumerate(shuf_a):
            if j not in a_map:
                a_map[j] = {}
            if n not in a_map[j]:
                a_map[j][n] = 0
            a_map[j][n] += 1
            if a_map[j][n] > a_map_max:
                a_map_max = a_map[j][n]

        shuf_b = seq_b[:]
        random.shuffle(shuf_b)
        for j, n in enumerate(shuf_b):
            if j not in b_map:
                b_map[j] = {}
            if n not in b_map[j]:
                b_map[j][n] = 0
            b_map[j][n] += 1
            if b_map[j][n] > b_map_max:
                b_map_max = b_map[j][n]

    for x in a_map:
        for y in a_map[x]:
            val = round(a_map[x][y] / a_map_max * 255)
            a_demo.putpixel((x, y + 50), (val,) * 3)

    for x in b_map:
        for y in b_map[x]:
            val = round(b_map[x][y] / b_map_max * 255)
            b_demo.putpixel((x, y + 50), (val,) * 3)

    # 50x50 square of x vs y ordering counts over 1000 shuffles
    seq_a = list(range(50))
    seq_b = list(range(50))

    a_map = {}
    b_map = {}
    for i in range(N_TRIALS):
        shuf_a = [
            seq_a[cohort.cohort_shuffle(j, 50, seed + i)]
                for j in range(len(seq_a))
        ]
        for j, n in enumerate(shuf_a):
            if n not in a_map:
                a_map[n] = {}
            for m in shuf_a[j + 1:]:
                if m not in a_map[n]:
                    a_map[n][m] = 0
                a_map[n][m] += 1

        shuf_b = seq_b[:]
        random.shuffle(shuf_b)
        for j, n in enumerate(shuf_b):
            if n not in b_map:
                b_map[n] = {}
            for m in shuf_b[j + 1:]:
                if m not in b_map[n]:
                    b_map[n][m] = 0
                b_map[n][m] += 1

    for first in a_map:
        for second in a_map[first]:
            val = round(a_map[first][second] / N_TRIALS * 255)
            a_demo.putpixel((second + 50, first + 50), (val,) * 3)

    for first in b_map:
        for second in b_map[first]:
            val = round(b_map[first][second] / N_TRIALS * 255)
            b_demo.putpixel((second + 50, first + 50), (val,) * 3)

    a_demo.save(os.path.join("demos", "rng_shuf_anarchy.png"), format="png")
    b_demo.save(os.path.join("demos", "rng_shuf_builtin.png"), format="png")


def demo_distribute():
    """
    Creates images that demonstrate the distribute functionality and
    compare it against irreversible distribution using the built-in
    random module.
    """
    seed = 3129320881
    random.seed(seed)

    a_demo = PIL.Image.new("RGB", (100, 100))
    #b_demo = PIL.Image.new("RGB", (100, 100))
    # TODO: Build a similar demo using built-in RNG?

    # Twenty rows of distributions of 50 elements within 10 10-item segments
    # Roughness 0.5
    for y in range(20):
        segments = []
        for i in range(10):
            segments.append([])
        for item in range(50):
            s = cohort.distribution_segment(item, 50, 10, 10, 0.5, seed + y)
            segments[s].append(item)

        for x in range(100):
            si = x // 10
            sii = x % 10
            if sii < len(segments[si]):
                val = round((20 + segments[si][sii]) / 70 * 255)
                a_demo.putpixel((x, y), (val,) * 3)

    # Five lines each with different roughness values:
    for i, r in enumerate([0, 0.1, 0.3, 0.5, 0.7, 0.9, 1]):
        start = 25 + i * 7
        for y in range(start, start + 5):
            segments = []
            for i in range(10):
                segments.append([])
            for item in range(50):
                s = cohort.distribution_segment(
                    item,
                    50,
                    10,
                    10,
                    r,
                    seed + y
                )
                segments[s].append(item)

            for x in range(100):
                si = x // 10
                sii = x % 10
                if sii < len(segments[si]):
                    val = round((20 + segments[si][sii]) / 70 * 255)
                    a_demo.putpixel((x, y), (val,) * 3)

    # Ten lines showing the distribution portion for each of 100 100-slot bins
    # Roughness: 0.2
    for y in range(76, 86):
        ys = seed + y
        for x in range(100):
            here = cohort.distribution_portion(x, 5000, 100, 100, 0.2, ys)
            val = round(here / 100 * 255)
            a_demo.putpixel((x, y), (val,) * 3)

    # Roughness: 0.8
    for y in range(90, 100):
        ys = seed + y
        for x in range(100):
            here = cohort.distribution_portion(x, 5000, 100, 100, 0.8, ys)
            val = round(here / 100 * 255)
            a_demo.putpixel((x, y), (val,) * 3)

    a_demo.save(os.path.join("demos", "rng_dist_anarchy.png"), format="png")
    #b_demo.save(os.path.join("demos", "rng_dist_builtin.png"), format="png")


WHITE, RED, BLUE = [
    (255, 255, 255),
    (255, 0, 83),
    (0, 121, 210),
]


def pick_color(seed):
    """
    Simple independent distribution function.
    """
    x = rng.uniform(seed)
    if x < 1 / 100:
        return RED
    elif x < 6 / 100:
        return BLUE
    else:
        return WHITE


def demo_shuf_compare():
    """
    Creates an image demonstrating the difference between shuffling and
    independent-chance-based distribution schemes.
    """
    seed = 9328810312
    random.seed(seed)

    demo = PIL.Image.new("RGB", (110, 110))

    n = 0
    for xblock in range(10):
        for yblock in range(10):
            lseed = seed + xblock * 729837 + yblock * 92873
            if yblock < 5:
                items = [pick_color(lseed + i * 4879283) for i in range(100)]
            else:
                items = [RED] + [BLUE] * 5 + [WHITE] * (100 - 6)

            for lx in range(10):
                for ly in range(10):
                    x = xblock * 11 + lx
                    y = yblock * 11 + ly

                    # shift lower blocks down to create separation
                    if yblock >= 5:
                        y += 1

                    if yblock < 5:
                        color = items[lx * 10 + ly]
                    else:
                        color = items[
                            cohort.cohort_shuffle(n, 100, lseed) % 100
                        ]

                    # Increment anarchy N
                    n += 1

                    demo.putpixel((x, y), color)

    demo.save(os.path.join("demos", "rng_shuf_compare.png"), format="png")


def main():
    if not os.path.exists("demos"):
        os.mkdir("demos")

    demo_prng()
    demo_shuffle()
    demo_distribute()
    demo_shuf_compare()


if __name__ == "__main__":
    main()
