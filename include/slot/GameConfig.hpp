#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "slot/ReelStrip.hpp"
#include "slot/Symbol.hpp"

namespace slot {

// Money is kept in whole credits (integers), never floating point, so totals
// are exact. Floating point is only used for statistics.
using Credits = std::int64_t;

// Row index on each reel, left to right. {1,1,1,1,1} is the middle row.
using Payline = std::vector<std::size_t>;

class ConfigError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Everything that defines one game. Loaded from JSON so designers and
// mathematicians can change the game without touching code.
struct GameConfig {
    std::string name;
    std::size_t rows = 3;
    std::vector<Symbol> symbols;
    std::vector<ReelStrip> reels;
    std::vector<Payline> paylines;

    // linePays[symbol][count]: multiplier of the line bet for `count` of
    // `symbol` in a row from the leftmost reel. Each row has reels+1 entries.
    std::vector<std::vector<Credits>> linePays;

    // scatterPays[count]: multiplier of the total bet for `count` scatters
    // anywhere in the window. Has reels*rows+1 entries.
    std::vector<Credits> scatterPays;

    [[nodiscard]] std::size_t reelCount() const noexcept { return reels.size(); }
    [[nodiscard]] const Symbol& symbol(SymbolId id) const { return symbols.at(id); }
    [[nodiscard]] std::optional<SymbolId> findSymbol(std::string_view symbolName) const;
    [[nodiscard]] std::optional<SymbolId> firstOfKind(SymbolKind kind) const;

    // Throws ConfigError if the config cannot describe a playable game.
    void validate() const;
};

// Parse a game definition. Throws ConfigError on bad input.
GameConfig parseConfig(std::string_view jsonText);
GameConfig loadConfig(const std::filesystem::path& path);

}  // namespace slot
