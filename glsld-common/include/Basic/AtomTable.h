#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

#include <vector>
#include <unordered_map>

namespace glsld
{
    class AtomString
    {
    public:
        AtomString() = default;
        explicit AtomString(const char* p) : ptr(p)
        {
        }

        auto Get() const noexcept -> const char*
        {
            return ptr;
        }

        auto Equals(const char* other) const noexcept -> bool
        {
            return ptr ? strcmp(ptr, other) == 0 : false;
        }

        auto Str() const noexcept -> std::string
        {
            return ptr ? ptr : "";
        }
        auto StrView() const noexcept -> StringView
        {
            return ptr ? ptr : "";
        }

        auto GetHashCode() const noexcept -> size_t
        {
            return std::hash<const char*>{}(ptr);
        }

        operator const char*() noexcept
        {
            return ptr;
        }

    private:
        // A pointer of C-style string that's hosted by the AtomTable.
        const char* ptr = nullptr;
    };

    inline auto operator==(const AtomString& lhs, const char* rhs) noexcept -> bool
    {
        return lhs.Equals(rhs);
    }
    inline auto operator==(const char* lhs, const AtomString& rhs) noexcept -> bool
    {
        return rhs.Equals(lhs);
    }

    // NOTE this is only valid if both args are registered in the same AtomTable, either directly or indirectly.
    // Otherwise, the comparison always returns false even if the two strings are equal.
    inline auto operator==(const AtomString& lhs, const AtomString& rhs) noexcept -> bool
    {
        return lhs.Get() == rhs.Get();
    }

    class AtomTable
    {
    private:
        struct BufferPage
        {
            char* bufferBegin;
            char* bufferEnd;
            char* bufferCursor;
        };

        static constexpr size_t BufferPageSize           = 4096;
        static constexpr size_t LargeStringThresholdSize = 64;

        std::vector<BufferPage> pagedBuffers;
        std::vector<char*> largeBuffers;

        std::unordered_map<StringView, AtomString> atomLookup;

    public:
        AtomTable() = default;
        ~AtomTable();

        // Import all atoms from another AtomTable.
        // NOTE caller must make sure the lifetime of the other AtomTable is longer than this one.
        auto Import(const AtomTable& other) -> void;

        // Get an atom string from the table that matches the given string.
        // If the string is not found, a new atom string will be created and added to the table.
        auto GetAtom(StringView s) -> AtomString;

        // Get an atom string from the table that matches the given string.
        // If the string is not found, an empty atom string will be returned.
        auto GetAtom(StringView s) const -> AtomString;

    private:
        auto AddAtom(StringView s) -> AtomString;
        auto AllocateBufferPage() -> BufferPage;
    };
} // namespace glsld