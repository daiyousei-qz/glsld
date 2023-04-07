#pragma once
#include "Common.h"

#include <compare>

namespace glsld
{
    // A position in a text document expressed as (zero-based) line and character offset.
    struct TextPosition
    {
        int line      = 0;
        int character = 0;

        auto operator==(const TextPosition&) const -> bool                  = default;
        auto operator<=>(const TextPosition&) const -> std::strong_ordering = default;
    };

    // A range in a text document expressed as (zero-based) start and end positions.
    struct TextRange
    {
        // Inclusive start position
        TextPosition start;

        // Exclusive end position
        TextPosition end;

        TextRange() = default;
        TextRange(TextPosition start, TextPosition end) : start(start), end(end)
        {
            GLSLD_ASSERT(start <= end);
        }

        auto Contains(TextPosition position) const -> bool
        {
            return start <= position && position < end;
        }

        auto Overlaps(TextRange range) const -> bool
        {
            if (start < range.start) {
                return Contains(range.end);
            }
            else {
                return range.Contains(start);
            }
        }

        auto operator==(const TextRange&) const -> bool                  = default;
        auto operator<=>(const TextRange&) const -> std::strong_ordering = delete;
    };
} // namespace glsld