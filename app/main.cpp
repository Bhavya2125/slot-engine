// Command-line front end.
//
//   slot spin     [--config FILE] [--seed N] [--bet N]
//   slot simulate [--config FILE] [--seed N] [--spins N]

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "slot/GameConfig.hpp"
#include "slot/Rng.hpp"
#include "slot/Simulator.hpp"
#include "slot/SlotMachine.hpp"

namespace {

struct Options {
    std::string command;
    std::string configPath = SLOT_DEFAULT_CONFIG;
    std::uint64_t seed = std::random_device{}();
    std::uint64_t spins = 1'000'000;
    slot::Credits bet = 1;
};

void printUsage() {
    std::cout << "usage:\n"
                 "  slot spin     [--config FILE] [--seed N] [--bet N]\n"
                 "  slot simulate [--config FILE] [--seed N] [--spins N]\n";
}

Options parseArgs(int argc, char** argv) {
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    if (args.empty()) throw std::invalid_argument("missing command");

    Options opts;
    opts.command = args[0];
    for (std::size_t i = 1; i < args.size(); ++i) {
        const auto flag = args[i];
        if (i + 1 >= args.size()) throw std::invalid_argument("missing value for " + std::string(flag));
        const std::string value(args[++i]);
        if (flag == "--config") opts.configPath = value;
        else if (flag == "--seed") opts.seed = std::stoull(value);
        else if (flag == "--spins") opts.spins = std::stoull(value);
        else if (flag == "--bet") opts.bet = std::stoll(value);
        else throw std::invalid_argument("unknown option " + std::string(flag));
    }
    return opts;
}

void printSpin(const slot::SlotMachine& machine, const slot::SpinResult& result) {
    const auto& config = machine.config();
    for (std::size_t row = 0; row < result.window.rows(); ++row) {
        for (std::size_t reel = 0; reel < result.window.reels(); ++reel) {
            std::cout << std::left << std::setw(9) << config.symbol(result.window.at(reel, row)).name;
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    for (const auto& win : result.lineWins) {
        std::cout << "line " << std::setw(2) << win.line + 1 << ": " << win.count << " x "
                  << config.symbol(win.symbol).name << " pays " << win.win << '\n';
    }
    if (result.scatter) {
        std::cout << "scatter: " << result.scatter->count << " pays " << result.scatter->win << '\n';
    }
    std::cout << "bet " << result.totalBet << ", won " << result.totalWin << '\n';
}

void printStats(const slot::SimulationStats& stats, std::uint64_t seed, double seconds) {
    std::cout << std::fixed << std::setprecision(4)
              << "spins          " << stats.spins << '\n'
              << "seed           " << seed << '\n'
              << "RTP            " << stats.rtp() * 100 << "% (+/- " << stats.rtpConfidence95() * 100 << "% at 95%)\n"
              << "hit frequency  " << stats.hitFrequency() * 100 << "% (1 in " << std::setprecision(2)
              << (stats.hits ? 1.0 / stats.hitFrequency() : 0.0) << ")\n"
              << std::setprecision(4)
              << "scatter hits   " << stats.scatterHits << '\n'
              << "std deviation  " << stats.stdDev() << " x bet per spin\n"
              << "max win        " << stats.maxWin << " credits\n"
              << std::setprecision(2)
              << "time           " << seconds << " s (" << static_cast<double>(stats.spins) / seconds / 1e6
              << "M spins/s)\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options opts = parseArgs(argc, argv);
        const slot::SlotMachine machine(slot::loadConfig(opts.configPath));

        if (opts.command == "spin") {
            slot::Mt19937Rng rng(opts.seed);
            std::cout << machine.config().name << " (seed " << opts.seed << ")\n\n";
            printSpin(machine, machine.spin(rng, opts.bet));
        } else if (opts.command == "simulate") {
            const auto start = std::chrono::steady_clock::now();
            const auto stats = slot::simulate(machine, opts.spins, opts.seed);
            const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
            std::cout << machine.config().name << "\n\n";
            printStats(stats, opts.seed, elapsed.count());
        } else {
            throw std::invalid_argument("unknown command " + opts.command);
        }
        return EXIT_SUCCESS;
    } catch (const std::invalid_argument& e) {
        std::cerr << "error: " << e.what() << "\n\n";
        printUsage();
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
