#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "TestGames.hpp"
#include "slot/Rng.hpp"
#include "slot/SlotMachine.hpp"

using namespace slot;
using namespace slot::test;

// Reel strip used by fiveByThree(): {A, B, WILD, SCAT, B, A, A, B}
//                          index:    0  1  2     3     4  5  6  7

TEST(SlotMachine, WindowShowsThreeConsecutiveStops) {
    const SlotMachine machine(fiveByThree());
    const Window w = machine.windowAt({0, 1, 2, 3, 4});
    EXPECT_EQ(w.at(0, 0), A);
    EXPECT_EQ(w.at(0, 1), B);
    EXPECT_EQ(w.at(0, 2), WILD);
    EXPECT_EQ(w.at(4, 0), B);
    EXPECT_EQ(w.at(4, 2), A);
}

TEST(SlotMachine, WindowWrapsAroundTheStrip) {
    const SlotMachine machine(fiveByThree());
    const Window w = machine.windowAt({7, 7, 7, 7, 7});
    EXPECT_EQ(w.at(0, 0), B);  // stop 7
    EXPECT_EQ(w.at(0, 1), A);  // stop 0
    EXPECT_EQ(w.at(0, 2), B);  // stop 1
}

TEST(SlotMachine, EvaluatesKnownStops) {
    const SlotMachine machine(fiveByThree());
    // Stop 4 puts A (index 5) on the middle row of every reel: 5 x A.
    const SpinResult r = machine.evaluateStops({4, 4, 4, 4, 4}, 2);
    EXPECT_EQ(r.totalBet, 2);
    ASSERT_EQ(r.lineWins.size(), 1u);
    EXPECT_EQ(r.lineWins[0].count, 5u);
    EXPECT_EQ(r.totalWin, 200 * 2);
}

TEST(SlotMachine, TotalWinAddsLineAndScatterWins) {
    const SlotMachine machine(fiveByThree());
    // Stop 2 shows WILD, SCAT, B on each reel: 5 scatters, middle row all SCAT.
    const SpinResult r = machine.evaluateStops({2, 2, 2, 2, 2}, 1);
    EXPECT_TRUE(r.lineWins.empty());
    ASSERT_TRUE(r.scatter);
    EXPECT_EQ(r.scatter->count, 5u);
    EXPECT_EQ(r.totalWin, 100);
}

TEST(SlotMachine, SpinUsesTheRng) {
    const SlotMachine machine(fiveByThree());
    SequenceRng rng({4, 4, 4, 4, 4});
    const SpinResult r = machine.spin(rng, 1);
    EXPECT_EQ(r.stops, (std::vector<std::size_t>{4, 4, 4, 4, 4}));
    EXPECT_EQ(r.totalWin, 200);
}

TEST(SlotMachine, RejectsBadInput) {
    const SlotMachine machine(fiveByThree());
    EXPECT_THROW((void)machine.windowAt({0, 0}), std::invalid_argument);
    EXPECT_THROW((void)machine.evaluateStops({0, 0, 0, 0, 0}, 0), std::invalid_argument);
}

TEST(SlotMachine, RejectsInvalidConfig) {
    GameConfig broken = fiveByThree();
    broken.paylines.clear();
    EXPECT_THROW(SlotMachine{std::move(broken)}, ConfigError);
}
