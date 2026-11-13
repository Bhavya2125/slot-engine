#include <gtest/gtest.h>

#include <vector>

#include "TestGames.hpp"
#include "slot/Evaluator.hpp"

using namespace slot;
using namespace slot::test;

namespace {

class EvaluatorTest : public ::testing::Test {
protected:
    GameConfig config = fiveByThree();
    Evaluator evaluator{config};

    std::optional<LineWin> line(std::vector<SymbolId> symbols) const { return evaluator.evaluateLine(symbols); }

    // Window whose middle row is `middle`; other rows are filled with B, A
    // so they never form a line of their own on the single payline.
    Window windowWithMiddle(const std::vector<SymbolId>& middle) const {
        Window w(5, 3);
        for (std::size_t reel = 0; reel < 5; ++reel) {
            w.set(reel, 0, B);
            w.set(reel, 1, middle[reel]);
            w.set(reel, 2, A);
        }
        return w;
    }
};

}  // namespace

TEST_F(EvaluatorTest, ThreeOfAKindPays) {
    const auto win = line({A, A, A, B, B});
    ASSERT_TRUE(win);
    EXPECT_EQ(win->symbol, A);
    EXPECT_EQ(win->count, 3u);
    EXPECT_EQ(win->win, 10);
}

TEST_F(EvaluatorTest, FiveOfAKindPaysTopPrize) {
    const auto win = line({A, A, A, A, A});
    ASSERT_TRUE(win);
    EXPECT_EQ(win->count, 5u);
    EXPECT_EQ(win->win, 200);
}

TEST_F(EvaluatorTest, TwoOfAKindPaysNothing) {
    EXPECT_FALSE(line({A, A, B, B, A}));
}

TEST_F(EvaluatorTest, MatchMustStartOnFirstReel) {
    EXPECT_FALSE(line({B, A, A, A, A}));
}

TEST_F(EvaluatorTest, WildSubstitutesInTheMiddle) {
    const auto win = line({A, WILD, A, A, B});
    ASSERT_TRUE(win);
    EXPECT_EQ(win->symbol, A);
    EXPECT_EQ(win->count, 4u);
    EXPECT_EQ(win->win, 50);
}

TEST_F(EvaluatorTest, LeadingWildsJoinTheFirstSymbol) {
    // 2 wilds alone pay nothing; read as A they make 4 of a kind.
    const auto win = line({WILD, WILD, A, A, B});
    ASSERT_TRUE(win);
    EXPECT_EQ(win->symbol, A);
    EXPECT_EQ(win->count, 4u);
}

TEST_F(EvaluatorTest, PaysTheBetterOfWildRunAndSymbolRun) {
    // 3 wilds pay 20; read as B they make 4 x B, which pays only 5.
    const auto win = line({WILD, WILD, WILD, B, A});
    ASSERT_TRUE(win);
    EXPECT_EQ(win->symbol, WILD);
    EXPECT_EQ(win->count, 3u);
    EXPECT_EQ(win->win, 20);
}

TEST_F(EvaluatorTest, AllWildsPayWildPrize) {
    const auto win = line({WILD, WILD, WILD, WILD, WILD});
    ASSERT_TRUE(win);
    EXPECT_EQ(win->symbol, WILD);
    EXPECT_EQ(win->win, 400);
}

TEST_F(EvaluatorTest, WildDoesNotReplaceScatter) {
    EXPECT_FALSE(line({SCAT, SCAT, SCAT, A, A}));
    EXPECT_FALSE(line({WILD, SCAT, A, A, A}));
}

TEST_F(EvaluatorTest, LineWinsUsePaylineRowsAndBet) {
    const Window w = windowWithMiddle({A, A, A, A, B});
    const auto wins = evaluator.lineWins(w, 3);
    ASSERT_EQ(wins.size(), 1u);
    EXPECT_EQ(wins[0].line, 0u);
    EXPECT_EQ(wins[0].win, 50 * 3);
}

TEST_F(EvaluatorTest, ScattersPayAnywhereTimesTotalBet) {
    Window w = windowWithMiddle({B, A, B, A, B});
    w.set(0, 0, SCAT);
    w.set(2, 2, SCAT);
    w.set(4, 0, SCAT);
    const auto scatter = evaluator.scatterWin(w, 10);
    ASSERT_TRUE(scatter);
    EXPECT_EQ(scatter->count, 3u);
    EXPECT_EQ(scatter->win, 5 * 10);
}

TEST_F(EvaluatorTest, TwoScattersPayNothing) {
    Window w = windowWithMiddle({B, A, B, A, B});
    w.set(0, 0, SCAT);
    w.set(1, 0, SCAT);
    EXPECT_FALSE(evaluator.scatterWin(w, 10));
}
