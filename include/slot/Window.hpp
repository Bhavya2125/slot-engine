#pragma once

#include <cstddef>
#include <vector>

#include "slot/Symbol.hpp"

namespace slot {

// The visible grid after a spin: `reels` columns by `rows` rows.
// Stored as one contiguous vector (column-major) instead of a vector of
// vectors, so a spin makes one allocation and reads are cache-friendly.
class Window {
public:
    Window(std::size_t reels, std::size_t rows) : reels_(reels), rows_(rows), cells_(reels * rows) {}

    [[nodiscard]] SymbolId at(std::size_t reel, std::size_t row) const { return cells_.at(index(reel, row)); }
    void set(std::size_t reel, std::size_t row, SymbolId symbol) { cells_.at(index(reel, row)) = symbol; }

    [[nodiscard]] std::size_t reels() const noexcept { return reels_; }
    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] const std::vector<SymbolId>& cells() const noexcept { return cells_; }

private:
    [[nodiscard]] std::size_t index(std::size_t reel, std::size_t row) const noexcept { return reel * rows_ + row; }

    std::size_t reels_;
    std::size_t rows_;
    std::vector<SymbolId> cells_;
};

}  // namespace slot
