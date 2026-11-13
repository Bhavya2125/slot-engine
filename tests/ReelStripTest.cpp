#include <gtest/gtest.h>

#include <stdexcept>

#include "slot/ReelStrip.hpp"

using slot::ReelStrip;

TEST(ReelStrip, ReturnsSymbolAtPosition) {
    const ReelStrip strip({4, 5, 6});
    EXPECT_EQ(strip.size(), 3u);
    EXPECT_EQ(strip.at(0), 4);
    EXPECT_EQ(strip.at(2), 6);
}

TEST(ReelStrip, WrapsAroundPastTheEnd) {
    const ReelStrip strip({4, 5, 6});
    EXPECT_EQ(strip.at(3), 4);
    EXPECT_EQ(strip.at(4), 5);
    EXPECT_EQ(strip.at(300), 4);
}

TEST(ReelStrip, CountsSymbolFrequency) {
    const ReelStrip strip({1, 2, 1, 1, 3});
    EXPECT_EQ(strip.count(1), 3u);
    EXPECT_EQ(strip.count(3), 1u);
    EXPECT_EQ(strip.count(9), 0u);
}

TEST(ReelStrip, RejectsEmptyStrip) {
    EXPECT_THROW(ReelStrip({}), std::invalid_argument);
}
