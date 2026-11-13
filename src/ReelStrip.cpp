#include "slot/ReelStrip.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace slot {

ReelStrip::ReelStrip(std::vector<SymbolId> stops) : stops_(std::move(stops)) {
    if (stops_.empty()) {
        throw std::invalid_argument("a reel strip needs at least one stop");
    }
}

std::size_t ReelStrip::count(SymbolId symbol) const noexcept {
    return static_cast<std::size_t>(std::count(stops_.begin(), stops_.end(), symbol));
}

}  // namespace slot
