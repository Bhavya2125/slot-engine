#include "slot/Simulator.hpp"

#include <algorithm>
#include <cmath>

#include "slot/Rng.hpp"

namespace slot {

void SimulationStats::record(Credits bet, Credits win, bool scatterHit) {
    ++spins;
    totalBet += bet;
    totalWin += win;
    if (win > 0) ++hits;
    if (scatterHit) ++scatterHits;
    maxWin = std::max(maxWin, win);

    // Welford's online update for mean and variance.
    const double x = static_cast<double>(win) / static_cast<double>(bet);
    const double delta = x - meanReturn;
    meanReturn += delta / static_cast<double>(spins);
    m2 += delta * (x - meanReturn);
}

double SimulationStats::rtp() const noexcept {
    return totalBet == 0 ? 0.0 : static_cast<double>(totalWin) / static_cast<double>(totalBet);
}

double SimulationStats::hitFrequency() const noexcept {
    return spins == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(spins);
}

double SimulationStats::stdDev() const noexcept {
    return spins < 2 ? 0.0 : std::sqrt(m2 / static_cast<double>(spins - 1));
}

double SimulationStats::rtpConfidence95() const noexcept {
    return spins < 2 ? 0.0 : 1.96 * stdDev() / std::sqrt(static_cast<double>(spins));
}

SimulationStats simulate(const SlotMachine& machine, std::uint64_t spins, std::uint64_t seed, Credits betPerLine) {
    Mt19937Rng rng(seed);
    SimulationStats stats;
    for (std::uint64_t i = 0; i < spins; ++i) {
        const SpinResult result = machine.spin(rng, betPerLine);
        stats.record(result.totalBet, result.totalWin, result.scatter.has_value());
    }
    return stats;
}

}  // namespace slot
