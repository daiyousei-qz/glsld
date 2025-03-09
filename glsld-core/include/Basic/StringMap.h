#pragma once
#include "Basic/StringView.h"

#include <map>
#include <unordered_map>
#include <string>

namespace glsld
{
    namespace detail
    {
        template <typename BaseMapType, bool IsMap>
        class BasicStringMap
        {
        private:
            BaseMapType baseMap;

        public:
            using IteratorType      = typename BaseMapType::iterator;
            using ConstIteratorType = typename BaseMapType::const_iterator;
            using ValueType         = typename BaseMapType::value_type;

            BasicStringMap() = default;
            BasicStringMap(std::initializer_list<ValueType> ilist) : baseMap(ilist)
            {
            }
            ~BasicStringMap() = default;

            BasicStringMap(const BasicStringMap&)                    = default;
            BasicStringMap(BasicStringMap&&)                         = default;
            auto operator=(const BasicStringMap&) -> BasicStringMap& = default;
            auto operator=(BasicStringMap&&) -> BasicStringMap&      = default;

            [[nodiscard]] auto Empty() const noexcept -> bool
            {
                return baseMap.empty();
            }

            [[nodiscard]] auto Size() const noexcept -> size_t
            {
                return baseMap.size();
            }

            [[nodiscard]] auto Find(StringView key) -> IteratorType
            {
                return baseMap.template find<StringView>(key);
            }
            [[nodiscard]] auto Find(StringView key) const -> ConstIteratorType
            {
                return baseMap.template find<StringView>(key);
            }

            [[nodiscard]] auto EqualRange(StringView key) -> std::pair<IteratorType, IteratorType>
            {
                return baseMap.template equal_range<StringView>(key);
            }
            [[nodiscard]] auto EqualRange(StringView key) const -> std::pair<ConstIteratorType, ConstIteratorType>
            {
                return baseMap.template equal_range<StringView>(key);
            }

            auto Insert(const ValueType& value)
            {
                return baseMap.insert(value);
            }
            auto Insert(ValueType&& value)
            {
                return baseMap.insert(std::move(value));
            }
            template <typename InputIt>
            auto Insert(InputIt first, InputIt last) -> void
            {
                baseMap.insert(first, last);
            }
            auto Insert(std::initializer_list<ValueType> ilist) -> void
            {
                baseMap.insert(ilist);
            }

            auto Erase(StringView key) -> IteratorType
            {
                if (auto it = Find(key); it != end()) {
                    return baseMap.erase(it);
                }

                return baseMap.end();
            }
            auto Erase(IteratorType it) -> IteratorType
            {
                return baseMap.erase(it);
            }

            auto Clear() noexcept -> void
            {
                baseMap.clear();
            }

            [[nodiscard]] auto begin() noexcept -> IteratorType
            {
                return baseMap.begin();
            }
            [[nodiscard]] auto begin() const noexcept -> ConstIteratorType
            {
                return baseMap.begin();
            }
            [[nodiscard]] auto end() noexcept -> IteratorType
            {
                return baseMap.end();
            }
            [[nodiscard]] auto end() const noexcept -> ConstIteratorType
            {
                return baseMap.end();
            }

            auto operator[](StringView key) -> decltype(auto)
                requires IsMap
            {
                // Since transparent operator[] needs c++26, we have to implement it manually
                auto it = Find(key);
                if (it == end()) {
                    return (Insert(ValueType{key, {}}).first->second);
                }
                return (it->second);
            }
            [[nodiscard]] auto operator==(const BasicStringMap& rhs) const noexcept -> bool
                requires std::equality_comparable<BaseMapType>
            {
                return baseMap == rhs.baseMap;
            }
            [[nodiscard]] auto operator<=>(const BasicStringMap& rhs) const noexcept -> std::strong_ordering
                requires std::three_way_comparable<BaseMapType>
            {
                return baseMap <=> rhs.baseMap;
            }
        };

        struct StringMapCompare
        {
            using is_transparent = void;

            constexpr auto operator()(StringView lhs, StringView rhs) const noexcept -> bool
            {
                return lhs < rhs;
            }
        };

        struct UnorderedStringMapHash
        {
            using is_transparent = void;

            auto operator()(StringView key) const noexcept -> size_t
            {
                return key.GetHashCode();
            }
        };

        struct UnorderedStringMapKeyEqual
        {
            using is_transparent = void;

            auto operator()(StringView lhs, StringView rhs) const noexcept -> bool
            {
                return lhs == rhs;
            }
        };
    } // namespace detail

    template <typename T>
    using StringMap = detail::BasicStringMap<std::map<std::string, T, detail::StringMapCompare>, true>;

    template <typename T>
    using StringMultiMap = detail::BasicStringMap<std::multimap<std::string, T, detail::StringMapCompare>, true>;

    template <typename T>
    using UnorderedStringMap = detail::BasicStringMap<
        std::unordered_map<std::string, T, detail::UnorderedStringMapHash, detail::UnorderedStringMapKeyEqual>, true>;

    template <typename T>
    using UnorderedStringMultiMap = detail::BasicStringMap<
        std::unordered_multimap<std::string, T, detail::UnorderedStringMapHash, detail::UnorderedStringMapKeyEqual>,
        true>;

} // namespace glsld