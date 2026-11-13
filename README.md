# Slot Engine

A data-driven slot machine engine in modern C++ (C++20) for Linux. The game
(symbols, reel strips, paylines and paytable) lives in a JSON file. The engine
spins reels, evaluates line and scatter wins, and runs Monte Carlo simulations
to measure the game's return to player (RTP).

## Build and run (Linux / WSL)

Needs GCC 11+ or Clang 14+, CMake 3.25+ and Ninja. GoogleTest and
nlohmann/json are downloaded by CMake.

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release

./build/release/slot spin --seed 7
./build/release/slot simulate --spins 10000000 --seed 1
```

Presets:

| Preset    | What it is for |
|-----------|----------------|
| `debug`   | Day-to-day development and gdb |
| `release` | Simulations (optimised) |
| `asan`    | Debug build with AddressSanitizer and UndefinedBehaviorSanitizer |
| `ci`      | Release with debug info; warnings are errors |

Memory check with Valgrind:

```bash
cmake --preset debug && cmake --build --preset debug
valgrind --leak-check=full ./build/debug/tests/slot_tests
```

## Current results

`config/classic_5x3.json`: 5 reels × 3 rows, 10 paylines, 32 stops per reel,
one wild and one scatter. 20M spins per seed, release build, GCC 13 on Ubuntu 24.04 (WSL2):

| Seed | RTP | 95% interval |
|------|-----|--------------|
| 1 | 96.32% | ± 0.13% |
| 2 | 96.24% | ± 0.13% |
| 3 | 96.29% | ± 0.13% |

Hit frequency 41.1% (1 in 2.43 spins). Single-threaded speed about
4.9M spins/s. 36 GoogleTest cases pass under the `release`, `ci` (warnings as
errors) and `asan` presets.

## How it works

```
config/*.json ──► GameConfig ──► SlotMachine ──spin(rng)──► SpinResult
                    (validated)      │                       (stops, window, wins)
                                     ├── ReelStrip × N  (circular symbol lists)
                                     └── Evaluator      (line wins + scatter wins)
Simulator: runs millions of spins ──► SimulationStats (RTP, hit rate, std dev, 95% CI)
```

| File | Responsibility |
|------|----------------|
| `include/slot/GameConfig.hpp` | Game definition, JSON loading and validation |
| `include/slot/ReelStrip.hpp`  | One reel: a circular list of symbols |
| `include/slot/Window.hpp`     | The visible grid, stored in one contiguous vector |
| `include/slot/Evaluator.hpp`  | Line wins (left to right, wild substitution) and scatter wins |
| `include/slot/SlotMachine.hpp`| Picks stops with an `Rng`, builds the window, totals the wins |
| `include/slot/Rng.hpp`        | RNG interface; Mersenne Twister for play, a fixed sequence for tests |
| `include/slot/Simulator.hpp`  | Monte Carlo RTP simulation (Welford mean and variance) |

### Game rules implemented

* **Line wins** pay left to right from the first reel: 3, 4 or 5 of a kind,
  multiplied by the line bet.
* **Wild** substitutes for any regular symbol. When a line starts with wilds
  (e.g. `WILD WILD SEVEN BAR`) it pays the better of the wild run and the
  symbol run.
* **Scatter** pays anywhere in the window, multiplied by the total bet. Wilds
  never replace scatters.
* **Money** is whole credits in `int64_t`, never floating point.

### Design decisions worth talking about

* **Data-driven.** Changing the maths means editing JSON, not code, so
  mathematicians and designers can tune a game without an engineer.
* **Injectable RNG.** `SlotMachine::spin` takes an `Rng&`, so tests feed a fixed
  sequence and get a known result.
* **Non-copyable `SlotMachine`.** `Evaluator` keeps a reference to the config the
  machine owns; copying or moving the machine would leave it dangling, so those
  operations are deleted.
* **Validation up front.** A bad config fails when it is loaded, with a clear
  message, never halfway through a spin.
* **Statistics.** RTP comes with a 95% confidence interval, so a simulation
  result can be compared with the theoretical value honestly.

## Roadmap

Each step is a chance to practise a C++ topic. Tick them off as you go.

### Week 1: core (done in this scaffold)
- [x] CMake project with presets, warnings, sanitizers and CI
- [x] Reel strips, window, paylines and paytable from JSON
- [x] Line evaluation with wilds; scatter pays
- [x] Monte Carlo RTP simulator and CLI
- [x] GoogleTest suite

### Week 2: maths and performance
- [ ] **Exact RTP by full-cycle enumeration.** Loop over every combination of
      stops (32^5 ≈ 33.5M) and compute the exact RTP. Assert that the simulation
      lands within its confidence interval of it.
- [ ] **Parallel simulation.** Split spins across `std::thread`s (one RNG per
      thread, seeded from the main seed), merge `SimulationStats`. Measure the
      speed-up.
- [ ] **Benchmarks** with Google Benchmark: spins per second before and after
      each optimisation.
- [ ] **Remove allocations from the hot loop.** Reuse the window and win
      vectors between spins; confirm with `valgrind --tool=massif` or heaptrack.
- [ ] **Profile** with `perf record` / `perf report` and write up what you found.

### Week 3: features
- [ ] **Free spins:** 3+ scatters award N free spins with a win multiplier.
      Model features as classes behind a `Feature` interface.
- [ ] **Volatility index** and a win-distribution histogram in the report.
- [ ] **Game state machine:** Idle → Spinning → Evaluating → Paying → Feature.
- [ ] **Front end** with SDL2 or SFML that animates the reels.
- [ ] **A second game config** (e.g. 243 ways-to-win) to prove the engine is generic.

## Interview checklist

Be ready to explain:

* Why money is `int64_t` and not `double`.
* How reel strip frequencies set the probabilities, and how to compute RTP by hand.
* Why the simulator uses Welford's algorithm and what the confidence interval means.
* Why `SlotMachine` is non-copyable (references, object lifetime, rule of five).
* What AddressSanitizer and Valgrind each catch.
* How you would make the simulator multi-threaded without data races.
