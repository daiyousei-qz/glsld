#include "Basic/AtomTable.h"

#include <algorithm>

namespace glsld
{
    AtomTable::AtomTable(const AtomTable* preambleAtomTable)
    {
        if (preambleAtomTable) {
            atomLookup = preambleAtomTable->atomLookup;
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
        char* atomPtr = reinterpret_cast<char*>(arena.Allocate(s.size() + 1));
        std::ranges::copy(s, atomPtr);
        atomPtr[s.size()] = '\0';

        return AtomString{atomPtr};
    }
} // namespace glsld