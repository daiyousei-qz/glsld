#pragma once
#include "Support/StringMap.h"

#include <set>
#include <unordered_set>

namespace glsld
{
    using StringSet = detail::BasicStringMap<std::set<std::string, detail::StringMapCompare>, true>;

    using StringMultiSet = detail::BasicStringMap<std::multiset<std::string, detail::StringMapCompare>, true>;

    using UnorderedStringSet = detail::BasicStringMap<
        std::unordered_set<std::string, detail::UnorderedStringMapHash, detail::UnorderedStringMapKeyEqual>, true>;

    using UnorderedStringMultiSet = detail::BasicStringMap<
        std::unordered_multiset<std::string, detail::UnorderedStringMapHash, detail::UnorderedStringMapKeyEqual>, true>;

} // namespace glsld