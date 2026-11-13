#include "slot/Evaluator.hpp"

#include <algorithm>

namespace slot {

Evaluator::Evaluator(const GameConfig& config)
    : config_(config),
      wild_(config.firstOfKind(SymbolKind::Wild)),
      scatter_(config.firstOfKind(SymbolKind::Scatter)) {}

Credits Evaluator::linePay(SymbolId symbol, std::size_t count) const {
    const auto& row = config_.linePays[symbol];
    return count < row.size() ? row[count] : 0;
}

// Pays left to right from the first reel. A wild stands in for any regular
// symbol. If a line starts with wilds, two readings are possible, e.g.
// WILD WILD SEVEN BAR: "2 wilds" or "3 sevens". The line pays the better one.
std::optional<LineWin> Evaluator::evaluateLine(const std::vector<SymbolId>& symbols) const {
    if (symbols.empty()) return std::nullopt;

    // Reading 1: the run of wilds on its own.
    std::size_t wildRun = 0;
    while (wildRun < symbols.size() && isWild(symbols[wildRun])) ++wildRun;
    const Credits wildPay = wildRun > 0 ? linePay(*wild_, wildRun) : 0;

    // Reading 2: the first non-wild symbol, extended by matches and wilds.
    Credits symbolPay = 0;
    std::size_t symbolRun = 0;
    SymbolId paying{};
    if (wildRun < symbols.size() && !isScatter(symbols[wildRun])) {
        paying = symbols[wildRun];
        symbolRun = wildRun;
        while (symbolRun < symbols.size() && (symbols[symbolRun] == paying || isWild(symbols[symbolRun]))) {
            ++symbolRun;
        }
        symbolPay = linePay(paying, symbolRun);
    }

    if (wildPay == 0 && symbolPay == 0) return std::nullopt;
    if (wildPay >= symbolPay) return LineWin{0, *wild_, wildRun, wildPay};
    return LineWin{0, paying, symbolRun, symbolPay};
}

std::vector<LineWin> Evaluator::lineWins(const Window& window, Credits betPerLine) const {
    std::vector<LineWin> wins;
    std::vector<SymbolId> onLine(window.reels());

    for (std::size_t line = 0; line < config_.paylines.size(); ++line) {
        const Payline& rows = config_.paylines[line];
        for (std::size_t reel = 0; reel < window.reels(); ++reel) {
            onLine[reel] = window.at(reel, rows[reel]);
        }
        if (auto win = evaluateLine(onLine)) {
            win->line = line;
            win->win *= betPerLine;
            wins.push_back(*win);
        }
    }
    return wins;
}

std::optional<ScatterWin> Evaluator::scatterWin(const Window& window, Credits totalBet) const {
    if (!scatter_) return std::nullopt;
    const auto& cells = window.cells();
    const auto count = static_cast<std::size_t>(std::count(cells.begin(), cells.end(), *scatter_));
    const Credits multiplier = count < config_.scatterPays.size() ? config_.scatterPays[count] : 0;
    if (multiplier == 0) return std::nullopt;
    return ScatterWin{count, multiplier * totalBet};
}

}  // namespace slot
