#include "Basic/AtomTable.h"

#include <algorithm>

namespace glsld
{
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
        char* atomPtr = reinterpret_cast<char*>(arena.Allocate(s.Size() + 1));
        std::ranges::copy(s, atomPtr);
        atomPtr[s.Size()] = '\0';

        return AtomString{atomPtr};
    }
} // namespace glsld