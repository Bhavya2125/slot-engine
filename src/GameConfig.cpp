#include "slot/GameConfig.hpp"

#include <fstream>
#include <limits>
#include <sstream>

#include <nlohmann/json.hpp>

namespace slot {

namespace {

using nlohmann::json;

SymbolKind parseKind(const std::string& kind) {
    if (kind == "regular") return SymbolKind::Regular;
    if (kind == "wild") return SymbolKind::Wild;
    if (kind == "scatter") return SymbolKind::Scatter;
    throw ConfigError("unknown symbol kind '" + kind + "' (expected regular, wild or scatter)");
}

SymbolId requireSymbol(const GameConfig& config, const std::string& symbolName) {
    if (auto id = config.findSymbol(symbolName)) return *id;
    throw ConfigError("unknown symbol '" + symbolName + "'");
}

// Reads {"3": 20, "4": 100} into table[count] = pay. The table is already
// sized; a count outside it is an error.
void readPayTable(const json& pays, std::vector<Credits>& table, const std::string& what) {
    for (const auto& [countText, pay] : pays.items()) {
        std::size_t count = 0;
        try {
            count = std::stoul(countText);
        } catch (const std::exception&) {
            throw ConfigError(what + ": '" + countText + "' is not a symbol count");
        }
        if (count == 0 || count >= table.size()) {
            throw ConfigError(what + ": count " + countText + " is out of range");
        }
        const auto value = pay.get<Credits>();
        if (value < 0) throw ConfigError(what + ": pays cannot be negative");
        table[count] = value;
    }
}

}  // namespace

std::optional<SymbolId> GameConfig::findSymbol(std::string_view symbolName) const {
    for (const auto& s : symbols) {
        if (s.name == symbolName) return s.id;
    }
    return std::nullopt;
}

std::optional<SymbolId> GameConfig::firstOfKind(SymbolKind kind) const {
    for (const auto& s : symbols) {
        if (s.kind == kind) return s.id;
    }
    return std::nullopt;
}

void GameConfig::validate() const {
    if (rows == 0) throw ConfigError("rows must be at least 1");
    if (reels.empty()) throw ConfigError("at least one reel is required");
    if (symbols.empty()) throw ConfigError("at least one symbol is required");
    if (symbols.size() > std::numeric_limits<SymbolId>::max()) throw ConfigError("too many symbols");
    if (paylines.empty()) throw ConfigError("at least one payline is required");

    for (std::size_t i = 0; i < symbols.size(); ++i) {
        if (symbols[i].id != i) throw ConfigError("symbol ids must match their position");
    }
    for (const auto& reel : reels) {
        for (SymbolId s : reel.stops()) {
            if (s >= symbols.size()) throw ConfigError("reel strip uses an unknown symbol id");
        }
    }
    for (std::size_t line = 0; line < paylines.size(); ++line) {
        const auto& rowsOnLine = paylines[line];
        if (rowsOnLine.size() != reels.size()) {
            throw ConfigError("payline " + std::to_string(line) + " must have one row per reel");
        }
        for (std::size_t row : rowsOnLine) {
            if (row >= rows) throw ConfigError("payline " + std::to_string(line) + " uses a row outside the window");
        }
    }
    if (linePays.size() != symbols.size()) throw ConfigError("linePays needs one row per symbol");
    for (const auto& row : linePays) {
        if (row.size() != reels.size() + 1) throw ConfigError("each linePays row needs reels+1 entries");
    }
    if (scatterPays.size() != reels.size() * rows + 1) {
        throw ConfigError("scatterPays needs reels*rows+1 entries");
    }
    std::size_t wilds = 0;
    std::size_t scatters = 0;
    for (const auto& s : symbols) {
        if (s.kind == SymbolKind::Wild) ++wilds;
        if (s.kind == SymbolKind::Scatter) ++scatters;
    }
    if (wilds > 1 || scatters > 1) throw ConfigError("at most one wild and one scatter symbol are supported");
}

GameConfig parseConfig(std::string_view jsonText) {
    json doc;
    try {
        doc = json::parse(jsonText);
    } catch (const json::parse_error& e) {
        throw ConfigError(std::string("invalid JSON: ") + e.what());
    }

    try {
        GameConfig config;
        config.name = doc.value("name", "Unnamed game");
        config.rows = doc.at("rows").get<std::size_t>();

        for (const auto& s : doc.at("symbols")) {
            Symbol symbol;
            symbol.id = static_cast<SymbolId>(config.symbols.size());
            symbol.name = s.at("name").get<std::string>();
            symbol.kind = parseKind(s.value("kind", "regular"));
            if (config.findSymbol(symbol.name)) throw ConfigError("duplicate symbol '" + symbol.name + "'");
            config.symbols.push_back(std::move(symbol));
        }

        for (const auto& strip : doc.at("reels")) {
            std::vector<SymbolId> stops;
            for (const auto& name : strip) stops.push_back(requireSymbol(config, name.get<std::string>()));
            if (stops.empty()) throw ConfigError("a reel strip needs at least one stop");
            config.reels.emplace_back(std::move(stops));
        }

        for (const auto& line : doc.at("paylines")) {
            config.paylines.push_back(line.get<Payline>());
        }

        const std::size_t reelCount = config.reels.size();
        config.linePays.assign(config.symbols.size(), std::vector<Credits>(reelCount + 1, 0));
        if (doc.contains("linePays")) {
            for (const auto& [name, pays] : doc.at("linePays").items()) {
                const SymbolId id = requireSymbol(config, name);
                readPayTable(pays, config.linePays[id], "linePays." + name);
            }
        }

        config.scatterPays.assign(reelCount * config.rows + 1, 0);
        if (doc.contains("scatterPays")) {
            readPayTable(doc.at("scatterPays"), config.scatterPays, "scatterPays");
        }

        config.validate();
        return config;
    } catch (const json::exception& e) {
        throw ConfigError(std::string("bad game config: ") + e.what());
    }
}

GameConfig loadConfig(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) throw ConfigError("cannot open config file " + path.string());
    std::ostringstream text;
    text << file.rdbuf();
    return parseConfig(text.str());
}

}  // namespace slot
