#pragma once
#include <fmt/format.h>

namespace glsld
{
    namespace detail
    {
        auto DebugPrintImpl(fmt::string_view fmt, fmt::format_args args) -> void;
    }

    template <typename... Args>
    inline auto Print(fmt::format_string<Args...> fmt, Args&&... args) -> void
    {
        const auto& vargs = fmt::make_format_args(args...);
        fmt::vprint(fmt, vargs);
    }

    template <typename... Args>
    inline auto Print(FILE* file, fmt::format_string<Args...> fmt, Args&&... args) -> void
    {
        const auto& vargs = fmt::make_format_args(args...);
        fmt::vprint(file, fmt, vargs);
    }

    template <typename... Args>
    inline auto DebugPrint(fmt::format_string<Args...> fmt, Args&&... args) -> void
    {
        const auto& vargs = fmt::make_format_args(args...);
        detail::DebugPrintImpl(fmt, vargs);
    }
} // namespace glsld