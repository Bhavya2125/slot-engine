#include <gtest/gtest.h>

#include <string>

#include "TestGames.hpp"
#include "slot/Simulator.hpp"

using namespace slot;

TEST(Simulator, SameSeedGivesSameResult) {
    const SlotMachine machine(test::fiveByThree());
    const auto first = simulate(machine, 20'000, 42);
    const auto second = simulate(machine, 20'000, 42);
    EXPECT_EQ(first.totalWin, second.totalWin);
    EXPECT_EQ(first.hits, second.hits);
}

TEST(Simulator, StatsAreConsistent) {
    const SlotMachine machine(test::fiveByThree());
    const auto s = simulate(machine, 20'000, 7, 2);
    EXPECT_EQ(s.spins, 20'000u);
    EXPECT_EQ(s.totalBet, 20'000 * 2);
    EXPECT_LE(s.hits, s.spins);
    EXPECT_LE(s.scatterHits, s.hits);
    EXPECT_GE(s.maxWin, 0);
    EXPECT_NEAR(s.meanReturn, s.rtp(), 1e-9);
}

// The coin-flip game has an exact RTP of 100%, worked out by hand.
// The simulation must land inside its own 95% confidence interval
// (widened to 4 standard errors so the test never flakes).
TEST(Simulator, ConvergesToKnownRtp) {
    const SlotMachine machine(test::coinFlips());
    const auto s = simulate(machine, 400'000, 2026);
    const double tolerance = s.rtpConfidence95() / 1.96 * 4.0;
    EXPECT_NEAR(s.rtp(), 1.0, tolerance);
    EXPECT_NEAR(s.hitFrequency(), 1.0 / 8.0, 0.005);
}

TEST(Simulator, ShippedGameHasSensibleRtp) {
    const SlotMachine machine(loadConfig(std::string(SLOT_CONFIG_DIR) + "/classic_5x3.json"));
    const auto s = simulate(machine, 200'000, 1);
    EXPECT_GT(s.rtp(), 0.80);
    EXPECT_LT(s.rtp(), 1.10);
}
