#pragma once

#include <type_traits>

namespace snl {
    template <typename... Hashes>
    size_t hashCombine(Hashes... hashes) {
        auto combine = [](std::size_t& seed, std::size_t hash) {
            seed ^= hash + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        };

        size_t seed = 0;
        (combine(seed, hashes), ...); // fold expression (C++17)
        return seed;
    }
}