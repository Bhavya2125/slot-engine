#pragma once

#include <cstdint>
#include <string>

namespace slot {

// Index into GameConfig::symbols. Small and trivially copyable so a reel
// window is a compact block of bytes.
using SymbolId = std::uint8_t;

enum class SymbolKind : std::uint8_t {
    Regular,  // pays on paylines, left to right
    Wild,     // substitutes for regular symbols on a payline
    Scatter,  // pays anywhere in the window, never on a payline
};

struct Symbol {
    SymbolId id{};
    std::string name;
    SymbolKind kind{SymbolKind::Regular};
};

}  // namespace slot
