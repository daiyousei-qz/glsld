#pragma once
#include <cstddef>

namespace glsld
{
    inline auto HashCombine(size_t seed, size_t value) -> size_t
    {
        return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }
} // namespace glsld