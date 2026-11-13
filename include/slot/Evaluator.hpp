#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "slot/GameConfig.hpp"
#include "slot/Window.hpp"

namespace slot {

struct LineWin {
    std::size_t line{};    // index into GameConfig::paylines
    SymbolId symbol{};     // symbol that paid (the wild itself for an all-wild run)
    std::size_t count{};   // symbols in a row from the left, wilds included
    Credits win{};

    bool operator==(const LineWin&) const = default;
};

struct ScatterWin {
    std::size_t count{};
    Credits win{};

    bool operator==(const ScatterWin&) const = default;
};

// Works out what a window pays. Holds no state between spins, so one
// Evaluator can be shared by many threads.
class Evaluator {
public:
    // Keeps a reference to `config`, which must outlive the Evaluator.
    explicit Evaluator(const GameConfig& config);

    [[nodiscard]] std::vector<LineWin> lineWins(const Window& window, Credits betPerLine) const;
    [[nodiscard]] std::optional<ScatterWin> scatterWin(const Window& window, Credits totalBet) const;

    // Best pay for one sequence of symbols read along a payline.
    // Public so it can be tested directly.
    [[nodiscard]] std::optional<LineWin> evaluateLine(const std::vector<SymbolId>& symbols) const;

private:
    [[nodiscard]] Credits linePay(SymbolId symbol, std::size_t count) const;
    [[nodiscard]] bool isWild(SymbolId symbol) const noexcept { return wild_ && *wild_ == symbol; }
    [[nodiscard]] bool isScatter(SymbolId symbol) const noexcept { return scatter_ && *scatter_ == symbol; }

    const GameConfig& config_;
    std::optional<SymbolId> wild_;
    std::optional<SymbolId> scatter_;
};

}  // namespace slot
