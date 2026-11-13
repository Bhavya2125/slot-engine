#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "slot/Evaluator.hpp"
#include "slot/GameConfig.hpp"
#include "slot/Rng.hpp"
#include "slot/Window.hpp"

namespace slot {

struct SpinResult {
    std::vector<std::size_t> stops;  // top-row stop index on each reel
    Window window;
    std::vector<LineWin> lineWins;
    std::optional<ScatterWin> scatter;
    Credits totalBet{};
    Credits totalWin{};
};

class SlotMachine {
public:
    // Validates the config; throws ConfigError if it is not playable.
    explicit SlotMachine(GameConfig config);

    // evaluator_ holds a reference into config_, so a copied or moved machine
    // would point at the wrong object. Forbid both.
    SlotMachine(const SlotMachine&) = delete;
    SlotMachine& operator=(const SlotMachine&) = delete;
    SlotMachine(SlotMachine&&) = delete;
    SlotMachine& operator=(SlotMachine&&) = delete;
    ~SlotMachine() = default;

    // One paid game: pick a stop per reel, build the window, evaluate it.
    [[nodiscard]] SpinResult spin(Rng& rng, Credits betPerLine) const;

    // Same as spin() but with the stops chosen by the caller. Deterministic.
    [[nodiscard]] SpinResult evaluateStops(const std::vector<std::size_t>& stops, Credits betPerLine) const;

    [[nodiscard]] Window windowAt(const std::vector<std::size_t>& stops) const;
    [[nodiscard]] Credits totalBet(Credits betPerLine) const noexcept;
    [[nodiscard]] const GameConfig& config() const noexcept { return config_; }

private:
    GameConfig config_;     // declared first: must be built before evaluator_
    Evaluator evaluator_;
};

}  // namespace slot
