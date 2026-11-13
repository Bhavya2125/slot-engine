#pragma once

#include <cstddef>
#include <vector>

#include "slot/Symbol.hpp"

namespace slot {

// One physical reel: a circular list of symbols. How often a symbol appears
// on the strip is what sets its probability, and so the game's maths.
class ReelStrip {
public:
    explicit ReelStrip(std::vector<SymbolId> stops);

    [[nodiscard]] std::size_t size() const noexcept { return stops_.size(); }

    // Symbol at `position`, wrapping past the end back to the start.
    [[nodiscard]] SymbolId at(std::size_t position) const noexcept {
        return stops_[position % stops_.size()];
    }

    // How many stops hold `symbol`.
    [[nodiscard]] std::size_t count(SymbolId symbol) const noexcept;

    [[nodiscard]] const std::vector<SymbolId>& stops() const noexcept { return stops_; }

private:
    std::vector<SymbolId> stops_;
};

}  // namespace slot
