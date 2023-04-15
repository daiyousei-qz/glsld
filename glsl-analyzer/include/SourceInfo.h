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

        auto operator==(const TextPosition&) const noexcept -> bool                  = default;
        auto operator<=>(const TextPosition&) const noexcept -> std::strong_ordering = default;
    };

    // A range in a text document expressed as (zero-based) start and end positions.
    struct TextRange
    {
        // Inclusive start position
        TextPosition start;

        // Exclusive end position
        TextPosition end;

        TextRange() = default;
        explicit TextRange(TextPosition start) : start(start), end(start)
        {
        }
        TextRange(TextPosition start, TextPosition end) : start(start), end(end)
        {
            GLSLD_ASSERT(start <= end);
        }

        auto IsEmpty() const noexcept -> bool
        {
            return start == end;
        }

        auto Contains(TextPosition position) const noexcept -> bool
        {
            return start <= position && position < end;
        }

        auto Overlaps(TextRange range) const noexcept -> bool
        {
            if (start < range.start) {
                return Contains(range.end);
            }
            else {
                return range.Contains(start);
            }
        }

        auto operator==(const TextRange&) const noexcept -> bool                  = default;
        auto operator<=>(const TextRange&) const noexcept -> std::strong_ordering = delete;
    };
} // namespace glsld