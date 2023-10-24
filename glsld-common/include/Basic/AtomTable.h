#pragma once
#include "Basic/Common.h"

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
    public:
        AtomTable() = default;
        ~AtomTable()
        {
            for (const auto& page : pagedBuffers) {
                delete[] page.bufferBegin;
            }
            for (const auto& buffer : largeBuffers) {
                delete[] buffer;
            }
        }

        // Import all atoms from another AtomTable.
        // NOTE when importing, we must make sure the lifetime of the other AtomTable is longer than this one.
        auto Import(const AtomTable& other) -> void
        {
            for (const auto& [key, value] : other.atomLookup) {
                atomLookup[key] = value;
            }
        }

        auto GetAtom(StringView s) -> AtomString
        {
            if (auto it = atomLookup.find(s); it != atomLookup.end()) {
                return it->second;
            }
            else {
                AtomString result = AddAtom(s);

                // NOTE here we cannot use parameter `s` directly, because its lifetime is not guaranteed.
                return atomLookup[result.StrView()] = result;
            }
        }

        auto GetAtom(StringView s) const -> AtomString
        {
            if (auto it = atomLookup.find(s); it != atomLookup.end()) {
                return it->second;
            }
            else {
                return {};
            }
        }

    private:
        struct BufferPage
        {
            char* bufferBegin;
            char* bufferEnd;
            char* bufferCursor;
        };

        static constexpr size_t BufferPageSize           = 4096;
        static constexpr size_t LargeStringThresholdSize = 64;

        auto AddAtom(StringView s) -> AtomString
        {
            char* atomPtr = nullptr;
            if (s.Size() < LargeStringThresholdSize) {
                BufferPage* currentPage = nullptr;
                if (pagedBuffers.empty()) {
                    currentPage = &pagedBuffers.emplace_back(AllocateBufferPage());
                }
                else {
                    currentPage = &pagedBuffers.back();
                }

                if (currentPage->bufferEnd - currentPage->bufferCursor < s.Size() + 1) {
                    currentPage = &pagedBuffers.emplace_back(AllocateBufferPage());
                }

                GLSLD_ASSERT(currentPage->bufferEnd - currentPage->bufferCursor >= s.Size() + 1);
                atomPtr = currentPage->bufferCursor;
                currentPage->bufferCursor += s.Size() + 1;
            }
            else {
                atomPtr = largeBuffers.emplace_back(new char[s.Size() + 1]);
            }

            std::ranges::copy(s, atomPtr);
            atomPtr[s.Size()] = '\0';
            return AtomString{atomPtr};
        }

        auto AllocateBufferPage() -> BufferPage
        {
            char* data = new char[BufferPageSize];
            return BufferPage{
                .bufferBegin  = data,
                .bufferEnd    = data + BufferPageSize,
                .bufferCursor = data,
            };
        }

        std::vector<BufferPage> pagedBuffers;
        std::vector<char*> largeBuffers;

        std::unordered_map<StringView, AtomString> atomLookup;
    };
} // namespace glsld