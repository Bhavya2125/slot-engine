#include <gtest/gtest.h>

#include <string>

#include "TestGames.hpp"
#include "slot/GameConfig.hpp"

using namespace slot;

namespace {

// A minimal valid game: 3 reels x 1 row, one line.
std::string minimalJson(const std::string& override = "") {
    std::string json = R"({
      "name": "mini",
      "rows": 1,
      "symbols": [{"name": "W", "kind": "wild"}, {"name": "S", "kind": "scatter"}, {"name": "A"}],
      "paylines": [[0, 0, 0]],
      "linePays": {"A": {"3": 10}},
      "scatterPays": {"3": 5},
      "reels": [["A", "W", "S"], ["A", "S"], ["A", "W"]]
    })";
    if (!override.empty()) json = override;
    return json;
}

}  // namespace

TEST(GameConfig, ParsesValidJson) {
    const GameConfig c = parseConfig(minimalJson());
    EXPECT_EQ(c.name, "mini");
    EXPECT_EQ(c.rows, 1u);
    ASSERT_EQ(c.symbols.size(), 3u);
    EXPECT_EQ(c.symbols[0].kind, SymbolKind::Wild);
    EXPECT_EQ(c.symbols[1].kind, SymbolKind::Scatter);
    EXPECT_EQ(c.symbols[2].kind, SymbolKind::Regular);
    ASSERT_EQ(c.reelCount(), 3u);
    EXPECT_EQ(c.reels[1].size(), 2u);
    EXPECT_EQ(c.linePays[2][3], 10);
    EXPECT_EQ(c.linePays[2][2], 0);
    EXPECT_EQ(c.scatterPays[3], 5);
}

TEST(GameConfig, FindsSymbolsByNameAndKind) {
    const GameConfig c = parseConfig(minimalJson());
    EXPECT_EQ(c.findSymbol("A"), SymbolId{2});
    EXPECT_FALSE(c.findSymbol("missing").has_value());
    EXPECT_EQ(c.firstOfKind(SymbolKind::Wild), SymbolId{0});
}

TEST(GameConfig, RejectsUnknownSymbolOnReel) {
    std::string json = minimalJson();
    json.replace(json.find(R"(["A", "S"])"), 10, R"(["A", "Q"])");
    EXPECT_THROW(parseConfig(json), ConfigError);
}

TEST(GameConfig, RejectsPaylineOutsideWindow) {
    std::string json = minimalJson();
    json.replace(json.find("[[0, 0, 0]]"), 11, "[[0, 1, 0]]");
    EXPECT_THROW(parseConfig(json), ConfigError);
}

TEST(GameConfig, RejectsPaylineWithWrongLength) {
    std::string json = minimalJson();
    json.replace(json.find("[[0, 0, 0]]"), 11, "[[0, 0]]");
    EXPECT_THROW(parseConfig(json), ConfigError);
}

TEST(GameConfig, RejectsPayCountLongerThanReels) {
    std::string json = minimalJson();
    json.replace(json.find(R"({"3": 10})"), 9, R"({"4": 10})");
    EXPECT_THROW(parseConfig(json), ConfigError);
}

TEST(GameConfig, RejectsBrokenJson) {
    EXPECT_THROW(parseConfig("{ not json"), ConfigError);
    EXPECT_THROW(parseConfig("{}"), ConfigError);  // missing required fields
}

TEST(GameConfig, ValidateCatchesHandBuiltMistakes) {
    GameConfig c = test::fiveByThree();
    EXPECT_NO_THROW(c.validate());
    c.scatterPays.pop_back();
    EXPECT_THROW(c.validate(), ConfigError);
}

TEST(GameConfig, ShippedGameLoads) {
    const GameConfig c = loadConfig(std::string(SLOT_CONFIG_DIR) + "/classic_5x3.json");
    EXPECT_EQ(c.reelCount(), 5u);
    EXPECT_EQ(c.rows, 3u);
    EXPECT_EQ(c.paylines.size(), 10u);
}
