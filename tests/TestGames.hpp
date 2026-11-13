#pragma once

// Small hand-built games whose results can be worked out on paper.

#include <vector>

#include "slot/GameConfig.hpp"

namespace slot::test {

// Symbol ids used by fiveByThree().
inline constexpr SymbolId WILD = 0;
inline constexpr SymbolId SCAT = 1;
inline constexpr SymbolId A = 2;  // high symbol
inline constexpr SymbolId B = 3;  // low symbol

// 5 reels x 3 rows, one payline across the middle row.
// Every reel is the strip {A, B, WILD, SCAT, B, A, A, B}.
inline GameConfig fiveByThree() {
    GameConfig c;
    c.name = "test 5x3";
    c.rows = 3;
    c.symbols = {{WILD, "WILD", SymbolKind::Wild},
                 {SCAT, "SCAT", SymbolKind::Scatter},
                 {A, "A", SymbolKind::Regular},
                 {B, "B", SymbolKind::Regular}};
    for (int i = 0; i < 5; ++i) c.reels.emplace_back(std::vector<SymbolId>{A, B, WILD, SCAT, B, A, A, B});
    c.paylines = {{1, 1, 1, 1, 1}};

    c.linePays.assign(4, std::vector<Credits>(6, 0));
    c.linePays[WILD] = {0, 0, 0, 20, 80, 400};
    c.linePays[A] = {0, 0, 0, 10, 50, 200};
    c.linePays[B] = {0, 0, 0, 2, 5, 20};

    c.scatterPays.assign(16, 0);
    c.scatterPays[3] = 5;
    c.scatterPays[4] = 20;
    c.scatterPays[5] = 100;
    return c;
}

// 3 reels x 1 row, each reel {X, Y}. Three X pays 8x on the single line.
// P(X X X) = 1/8, so the exact RTP is 8 * 1/8 = 100%.
inline GameConfig coinFlips() {
    GameConfig c;
    c.name = "coin flips";
    c.rows = 1;
    c.symbols = {{0, "X", SymbolKind::Regular}, {1, "Y", SymbolKind::Regular}};
    for (int i = 0; i < 3; ++i) c.reels.emplace_back(std::vector<SymbolId>{0, 1});
    c.paylines = {{0, 0, 0}};
    c.linePays = {{0, 0, 0, 8}, {0, 0, 0, 0}};
    c.scatterPays.assign(4, 0);
    return c;
}

}  // namespace slot::test
