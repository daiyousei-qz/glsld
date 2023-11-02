#include "Basic/AtomTable.h"

namespace glsld
{
    AtomTable::~AtomTable()
    {
        for (const auto& page : pagedBuffers) {
            delete[] page.bufferBegin;
        }
        for (const auto& buffer : largeBuffers) {
            delete[] buffer;
        }
    }

    auto AtomTable::Import(const AtomTable& other) -> void
    {
        for (const auto& [key, value] : other.atomLookup) {
            atomLookup[key] = value;
        }
    }

    auto AtomTable::GetAtom(StringView s) -> AtomString
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

    auto AtomTable::GetAtom(StringView s) const -> AtomString
    {
        if (auto it = atomLookup.find(s); it != atomLookup.end()) {
            return it->second;
        }
        else {
            return {};
        }
    }

    auto AtomTable::AddAtom(StringView s) -> AtomString
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

    auto AtomTable::AllocateBufferPage() -> BufferPage
    {
        char* data = new char[BufferPageSize];
        return BufferPage{
            .bufferBegin  = data,
            .bufferEnd    = data + BufferPageSize,
            .bufferCursor = data,
        };
    }
} // namespace glsld