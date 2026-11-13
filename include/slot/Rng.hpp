#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

namespace slot {

// Random source used to pick reel stops. An interface so tests can inject a
// fixed sequence and get a known window.
class Rng {
public:
    virtual ~Rng() = default;

    // Uniform integer in [0, bound). `bound` must be greater than zero.
    virtual std::uint32_t below(std::uint32_t bound) = 0;

protected:
    Rng() = default;
    Rng(const Rng&) = default;
    Rng& operator=(const Rng&) = default;
};

// Production RNG. 64-bit Mersenne Twister with an unbiased distribution.
// (A certified game would use an approved RNG; this is fine for simulation.)
class Mt19937Rng final : public Rng {
public:
    explicit Mt19937Rng(std::uint64_t seed) : engine_(seed) {}

    std::uint32_t below(std::uint32_t bound) override {
        std::uniform_int_distribution<std::uint32_t> dist(0, bound - 1);
        return dist(engine_);
    }

private:
    std::mt19937_64 engine_;
};

// Replays a fixed sequence of values (each taken modulo `bound`). For tests.
class SequenceRng final : public Rng {
public:
    explicit SequenceRng(std::vector<std::uint32_t> values) : values_(std::move(values)) {}

    std::uint32_t below(std::uint32_t bound) override {
        const std::uint32_t value = values_[next_ % values_.size()];
        ++next_;
        return value % bound;
    }

private:
    std::vector<std::uint32_t> values_;
    std::size_t next_ = 0;
};

}  // namespace slot
