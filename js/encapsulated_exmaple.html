<!doctype html>
<head>
    <meta charset="utf-8">
    <title>Encapsulated Anarchy JavaScript Example</title>
    <meta content="author" value="Peter Mawhorter">
    <!--script type="text/javascript" src="js/require.js"></script-->
    <script type="text/javascript">
        function define(reqs, module) { anarchy = module(reqs); }
    </script>
    <script type="text/javascript" src="js/anarchy.js"></script>
</head>
<body>

<h1>Shuffling vs. Independent Random Chance</h1>

<p>
This example shows the difference between using anarchy to shuffle a
distribution vs. using standard independent sampling. You can view the
source code of this page to see how the code is written.
</p>

<p>
On the left, each tile has an independent 2% chance of being blue and 8% chance
of being red. On the right, 2 out of the 100 tiles will always be blue, and an
additional 8 will always be red. One or the other might be more accurate in
different simulation contexts, and they also produce very different experiences
in a game context.
</p>

<div id="compare_demo_controls">
    <input id="compare_demo_seed" type="text" value="0"/>
    <input id="compare_demo_update" type="button" value="update"/>
    <input id="compare_demo_next" type="button" value="next"/>
</div>

<canvas id="compare_demo" width=800 height=300></canvas>

<script defer type="text/javascript">

// EDIT HERE
// The code inside this "<script>" tag (from line 39 to line 174) is all
// JavaScript code that you should edit to tweak this demo. You're free
// to throw most of it out and start from the basics to draw whatever you
// like on the canvas, or just tweak it a bit to draw something slightly
// different.

function enable_compare_demo() {
  // Grab the div that we want to work with
  let canvas = document.getElementById("compare_demo");

  let ctx = canvas.getContext("2d");

  let rect_size = 10;

  let colors = ["white", "red", "blue"];

  // Draws rectangle grid at the given x/y location using color values drawn
  // repeatedly from the given RNG function. That function must return a number
  // that's a valid index into the colors array.
  function draw_result(where, rng) {
    for (let i = 0; i < 100; ++i) {
      let r = rng()
      let x = i % 10;
      let y = Math.floor(i/10);
      let cx = where[0] + rect_size/2 + x * rect_size;
      let cy = where[1] + rect_size/2 + y * rect_size;
      ctx.fillStyle = colors[r];
      ctx.fillRect(cx, cy, rect_size, rect_size);
      ctx.strokeWidth = 0.5;
      ctx.strokeStyle = "black";
      ctx.strokeRect(cx, cy, rect_size, rect_size);
    }
  }

  // Creates RNG that spits out 0, 1, or 2 independently with 90%, 8%, and 2%
  // probabilities.
  function make_independent_rng(seed) {
    let val = 0;
    return function () {
      val = anarchy.prng(val, seed);
      let u = anarchy.udist(val);
      if (u <= 0.02) {
        return 2;
      } else if (u <= 0.1) {
        return 1;
      } else {
        return 0;
      }
    }
  }

  // Creates RNG that spits out 0, 1, or 2 based on shuffling 100 items, 90 of
  // which are 0's, 8 of which are 1's, and 2 of which are 2's. Repeats pattern
  // exactly after the 100th item.
  function make_shuffle_rng(seed) {
    let idx = 0;
    return function () {
      let val = anarchy.cohort_shuffle(idx, 100, seed);
      idx += 1;
      idx %= 100;
      if (val <= 1) {
        return 2;
      } else if (val <= 9) {
        return 1;
      } else {
        return 0;
      }
    }
  }


  // Shows results for the given seed by drawing four sampled results on the
  // left and four shuffled results on the right.
  function show_results(seed) {
    let base_seed = seed * 4;

    let pad = 100/3;
    for (let i = 0; i < 4; ++i) {
      let xo = (i % 2) * (100 + pad);
      let yo = Math.floor(i / 2) * (100 + pad);
      let irng = make_independent_rng(base_seed + i);
      let srng = make_shuffle_rng(base_seed + i);

      draw_result([pad + xo, pad + yo], irng);
      draw_result([200 + 4*pad + xo, pad + yo], srng);
    }

    // add dividing line
    ctx.strokeWidth = 2;
    ctx.strokeStyle = "black";
    ctx.beginPath();
    ctx.moveTo(300, 0);
    ctx.lineTo(300, 600);
    ctx.stroke();
  }

  // Function for picking up seed from input
  function get_seed() {
    let seed_input = document.getElementById("compare_demo_seed");
    let s = parseInt(seed_input.value);
    if (s == undefined) {
      s = 0;
      seed_input.value = s;
    }
    return s;
  }

  // Show initial results:
  show_results(get_seed())

  // Function for updating seed input
  function set_seed(x) {
    let seed_input = document.getElementById("compare_demo_seed");
    seed_input.value = x;
  }

  // Set up shuffle button to use current seed:
  let cmp_update_button = document.getElementById("compare_demo_update")
  cmp_update_button.addEventListener("click", function () {
    show_results(get_seed());
  });

  // Set up next button to increment and shuffle:
  let cmp_next_button = document.getElementById("compare_demo_next")
  cmp_next_button.addEventListener("click", function () {
    let seed = get_seed();
    seed += 1;
    set_seed(seed);
    show_results(seed);
  });
}

// Run all this stuff:
enable_compare_demo();
</script>
</body>
</html>
