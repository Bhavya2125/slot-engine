#pragma once

#include <cstdint>

#include "slot/GameConfig.hpp"
#include "slot/SlotMachine.hpp"

namespace slot {

// Results of a Monte Carlo run. RTP (return to player) is total won divided
// by total bet: 0.96 means the game pays back 96% over the long run.
struct SimulationStats {
    std::uint64_t spins = 0;
    Credits totalBet = 0;
    Credits totalWin = 0;
    std::uint64_t hits = 0;              // spins that paid anything
    std::uint64_t scatterHits = 0;       // spins with a scatter win
    Credits maxWin = 0;

    // Running mean and variance of (win / bet) per spin, via Welford's
    // algorithm, which stays accurate over hundreds of millions of spins.
    double meanReturn = 0.0;
    double m2 = 0.0;

    void record(Credits bet, Credits win, bool scatterHit);

    [[nodiscard]] double rtp() const noexcept;
    [[nodiscard]] double hitFrequency() const noexcept;
    [[nodiscard]] double stdDev() const noexcept;        // per spin, in units of the bet
    [[nodiscard]] double rtpConfidence95() const noexcept;  // +/- half-width of a 95% interval
};

// Plays `spins` games with a seeded RNG. The same seed gives the same result.
SimulationStats simulate(const SlotMachine& machine, std::uint64_t spins, std::uint64_t seed,
                         Credits betPerLine = 1);

}  // namespace slot
