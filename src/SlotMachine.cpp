#include "slot/SlotMachine.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

namespace slot {

namespace {
GameConfig validated(GameConfig config) {
    config.validate();
    return config;
}
}  // namespace

SlotMachine::SlotMachine(GameConfig config) : config_(validated(std::move(config))), evaluator_(config_) {}

Credits SlotMachine::totalBet(Credits betPerLine) const noexcept {
    return betPerLine * static_cast<Credits>(config_.paylines.size());
}

Window SlotMachine::windowAt(const std::vector<std::size_t>& stops) const {
    if (stops.size() != config_.reelCount()) {
        throw std::invalid_argument("need one stop per reel");
    }
    Window window(config_.reelCount(), config_.rows);
    for (std::size_t reel = 0; reel < config_.reelCount(); ++reel) {
        const ReelStrip& strip = config_.reels[reel];
        for (std::size_t row = 0; row < config_.rows; ++row) {
            window.set(reel, row, strip.at(stops[reel] + row));
        }
    }
    return window;
}

SpinResult SlotMachine::evaluateStops(const std::vector<std::size_t>& stops, Credits betPerLine) const {
    if (betPerLine <= 0) throw std::invalid_argument("bet per line must be positive");

    SpinResult result{stops, windowAt(stops), {}, std::nullopt, totalBet(betPerLine), 0};
    result.lineWins = evaluator_.lineWins(result.window, betPerLine);
    result.scatter = evaluator_.scatterWin(result.window, result.totalBet);

    for (const auto& win : result.lineWins) result.totalWin += win.win;
    if (result.scatter) result.totalWin += result.scatter->win;
    return result;
}

SpinResult SlotMachine::spin(Rng& rng, Credits betPerLine) const {
    std::vector<std::size_t> stops(config_.reelCount());
    for (std::size_t reel = 0; reel < config_.reelCount(); ++reel) {
        const auto size = config_.reels[reel].size();
        if (size > std::numeric_limits<std::uint32_t>::max()) throw std::length_error("reel strip too long");
        stops[reel] = rng.below(static_cast<std::uint32_t>(size));
    }
    return evaluateStops(stops, betPerLine);
}

}  // namespace slot
