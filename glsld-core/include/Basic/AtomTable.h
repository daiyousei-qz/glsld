#pragma once
#include "Basic/MemoryArena.h"
#include "Basic/StringView.h"

#include <unordered_map>

namespace glsld
{
    class AtomString
    {
    private:
        // A pointer of C-style string that's hosted by the AtomTable.
        const char* ptr = nullptr;

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
        auto Equals(StringView other) const noexcept -> bool
        {
            return ptr ? StringView{ptr} == other : other == "";
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
    };

    inline auto operator==(const AtomString& lhs, const char* rhs) noexcept -> bool
    {
        return lhs.Equals(rhs);
    }
    inline auto operator==(const char* lhs, const AtomString& rhs) noexcept -> bool
    {
        return rhs.Equals(lhs);
    }
    inline auto operator==(const AtomString& lhs, StringView rhs) noexcept -> bool
    {
        return lhs.Equals(rhs);
    }
    inline auto operator==(StringView lhs, const AtomString& rhs) noexcept -> bool
    {
        return rhs.Equals(lhs);
    }

    // NOTE this is only valid if both args are registered in the same AtomTable, either directly or indirectly.
    // Otherwise, the comparison always returns false even if the two strings are equal.
    inline auto operator==(const AtomString& lhs, const AtomString& rhs) noexcept -> bool
    {
        return lhs.Get() == rhs.Get();
    }

    class AtomTable final
    {
    private:
        BasicMemoryArena<false> arena;

        std::unordered_map<StringView, AtomString> atomLookup;

    public:
        AtomTable()  = default;
        ~AtomTable() = default;

        AtomTable(const AtomTable&)            = delete;
        AtomTable& operator=(const AtomTable&) = delete;

        AtomTable(AtomTable&&)            = default;
        AtomTable& operator=(AtomTable&&) = default;

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
    };
} // namespace glsld