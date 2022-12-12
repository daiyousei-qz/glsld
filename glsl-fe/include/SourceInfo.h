#pragma once
#include "Common.h"

#include <compare>

namespace glsld
{
    // A position into a text document.
    // NOTE both line and character are zero-based.
    struct TextPosition
    {
        int line      = 0;
        int character = 0;

        auto operator==(const TextPosition&) const -> bool                  = default;
        auto operator<=>(const TextPosition&) const -> std::strong_ordering = default;
    };

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
            if (position.line < start.line || (position.line == start.line && position.character < start.character)) {
                return false;
            }
            if (position.line > end.line || (position.line == end.line && position.character >= end.character)) {
                return false;
            }

            return true;
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