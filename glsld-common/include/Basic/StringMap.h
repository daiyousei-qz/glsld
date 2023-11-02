#pragma once
#include "Basic/StringView.h"

#include <map>
#include <unordered_map>
#include <string>

namespace glsld
{
    namespace detail
    {
        template <typename T, typename BaseMapType>
        class StringMapBase
        {
        private:
            BaseMapType baseMap;

        public:
            using IteratorType      = typename BaseMapType::iterator;
            using ConstIteratorType = typename BaseMapType::const_iterator;
            using ValueType         = typename BaseMapType::value_type;

            StringMapBase()  = default;
            ~StringMapBase() = default;

            StringMapBase(const StringMapBase&)                    = default;
            StringMapBase(StringMapBase&&)                         = default;
            auto operator=(const StringMapBase&) -> StringMapBase& = default;
            auto operator=(StringMapBase&&) -> StringMapBase&      = default;

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
                return baseMap.template find<StringView>(key.Str());
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

            [[nodiscard]] auto operator==(const StringMapBase& rhs) const noexcept -> bool
            {
                return baseMap == rhs.baseMap;
            }
            [[nodiscard]] auto operator<=>(const StringMapBase& rhs) const noexcept -> std::strong_ordering
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
    using StringMap = detail::StringMapBase<T, std::map<std::string, T, detail::StringMapCompare>>;

    template <typename T>
    using StringMultiMap = detail::StringMapBase<T, std::multimap<std::string, T, detail::StringMapCompare>>;

    template <typename T>
    using UnorderedStringMap = detail::StringMapBase<
        T, std::unordered_map<std::string, T, detail::UnorderedStringMapHash, detail::UnorderedStringMapKeyEqual>>;

    template <typename T>
    using UnorderedStringMultiMap = detail::StringMapBase<
        T, std::unordered_multimap<std::string, T, detail::UnorderedStringMapHash, detail::UnorderedStringMapKeyEqual>>;

} // namespace glsld