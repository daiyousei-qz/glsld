#pragma once
#include "Basic/Common.h"

#include <compare>

namespace glsld
{
    // An opaque identifier for a file.
    // - 0: Invalid file ID.
    // - -1: System preamble file ID.
    // - -2: User preamble file ID.
    // - Others: User file ID.
    class FileID
    {
    private:
        uint32_t id = 0;

        static constexpr uint32_t SystemPreambleID = static_cast<uint32_t>(-1);
        static constexpr uint32_t UserPreambleID   = static_cast<uint32_t>(-2);

    public:
        FileID() = default;

        operator bool() const noexcept
        {
            return IsValid();
        }

        auto IsValid() const noexcept -> bool
        {
            return id != 0;
        }
        auto IsSystemPreable() const noexcept -> bool
        {
            return id == SystemPreambleID;
        }
        auto IsUserPreamble() const noexcept -> bool
        {
            return id == UserPreambleID;
        }
        auto IsPreamble() const noexcept -> bool
        {
            return IsSystemPreable() || IsUserPreamble();
        }
        auto IsUserFile() const noexcept -> bool
        {
            return IsValid() && !IsPreamble();
        }

        auto GetValue() const noexcept -> uint32_t
        {
            return id;
        }

        static auto FromIndex(uint32_t index) noexcept -> FileID
        {
            FileID fileID;
            fileID.id = index;
            return fileID;
        }
        static auto SystemPreamble() noexcept -> FileID
        {
            return FromIndex(SystemPreambleID);
        }
        static auto UserPreamble() noexcept -> FileID
        {
            return FromIndex(UserPreambleID);
        }
    };

    inline auto operator==(const FileID& lhs, const FileID& rhs) noexcept -> bool
    {
        return lhs.GetValue() == rhs.GetValue();
    }

    // A position in a text document expressed as (zero-based) line and character offset.
    struct TextPosition
    {
        int line      = 0;
        int character = 0;

        auto operator==(const TextPosition&) const noexcept -> bool                  = default;
        auto operator<=>(const TextPosition&) const noexcept -> std::strong_ordering = default;

        static auto Min() noexcept -> TextPosition
        {
            return TextPosition{0, 0};
        }
        static auto Max() noexcept -> TextPosition
        {
            return TextPosition{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
        }
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

        auto GetNumLines() const noexcept -> int
        {
            return end.line - start.line + 1;
        }

        auto Contains(TextPosition position) const noexcept -> bool
        {
            return start <= position && position < end;
        }

        auto ContainsExtended(TextPosition position) const noexcept -> bool
        {
            return start <= position && position <= end;
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

    struct FileTextRange
    {
        FileID fileID;
        TextRange range;
    };
} // namespace glsld