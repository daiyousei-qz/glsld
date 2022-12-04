#pragma once
#include <cassert>
#include <span>

#include <fmt/format.h>

#define GLSLD_DEBUG 1
#define GLSLD_DEBUG_PARSER 1

#define GLSLD_ASSERT(EXPR) assert(EXPR)

#define GLSLD_UNREACHABLE() std::abort()
#define GLSLD_NO_IMPL() std::abort()

namespace glsld
{
    template <typename T, size_t Extent = std::dynamic_extent>
    using ArraySpan = std::span<T, Extent>;

    template <typename T, size_t Extent = std::dynamic_extent>
    using ArrayView = std::span<const T, Extent>;
} // namespace glsld