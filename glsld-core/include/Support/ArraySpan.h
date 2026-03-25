#pragma once
#include <algorithm>
#include <concepts>
#include <ranges>
#include <span>

namespace glsld
{
    template <typename T, size_t Extent = std::dynamic_extent>
    class ArraySpan : std::ranges::view_base
    {
    private:
        using SpanType = std::span<T, Extent>;

        SpanType span;

    public:
        using Iterator      = SpanType::iterator;
        using ConstIterator = SpanType::const_iterator;

        constexpr ArraySpan() = default;

        constexpr ArraySpan(T* p, size_t count) : span(p, count)
        {
        }
        constexpr ArraySpan(SpanType span) : span(span)
        {
        }
        constexpr ArraySpan(std::nullptr_t p) = delete;

        template <size_t N>
            requires(Extent == std::dynamic_extent || N == Extent)
        constexpr ArraySpan(T (&arr)[N]) : span(arr, N)
        {
        }

        template <size_t N>
            requires(Extent == std::dynamic_extent || N == Extent)
        constexpr ArraySpan(std::array<T, N>& arr) : span(arr.data(), N)
        {
        }

        template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
            requires(!std::is_convertible_v<End, size_t>)
        constexpr ArraySpan(It first, End last) : span(first, last)
        {
            // We let the underlying std::span constructor validate the range
        }

        template <std::ranges::contiguous_range R>
            requires requires(R& r) {
                { std::ranges::data(r) } -> std::convertible_to<T*>;
            }
        constexpr ArraySpan(R& r) : span(std::ranges::data(r), std::ranges::size(r))
        {
            // We let the underlying std::span constructor validate the range
        }

        template <std::ranges::contiguous_range R>
            requires requires(const R& r) {
                { std::ranges::data(r) } -> std::convertible_to<T*>;
            }
        constexpr ArraySpan(const R& r) : span(std::ranges::data(r), std::ranges::size(r))
        {
            // We let the underlying std::span constructor validate the range
        }

        [[nodiscard]] constexpr auto empty() const noexcept -> bool
        {
            return span.empty();
        }

        [[nodiscard]] constexpr auto size() const noexcept -> size_t
        {
            return span.size();
        }

        [[nodiscard]] constexpr auto front() const noexcept -> T&
        {
            return span.front();
        }

        [[nodiscard]] constexpr auto back() const noexcept -> T&
        {
            return span.back();
        }

        // Operations

        [[nodiscard]] constexpr auto Take(size_t n) const noexcept -> ArraySpan
        {
            return ArraySpan{span.subspan(0, n)};
        }
        [[nodiscard]] constexpr auto Drop(size_t n) const noexcept -> ArraySpan
        {
            return ArraySpan{span.subspan(n)};
        }
        template <typename Fn>
            requires std::predicate<Fn, T>
        [[nodiscard]] constexpr auto TakeUntil(Fn&& predicate) const noexcept -> ArraySpan
        {
            return ArraySpan{std::to_address(span.begin()),
                             std::to_address(std::ranges::find_if(span, std::forward<decltype(predicate)>(predicate)))};
        }
        template <typename Fn>
            requires std::predicate<Fn, T>
        [[nodiscard]] constexpr auto DropUntil(Fn&& predicate) const noexcept -> ArraySpan
        {
            return ArraySpan{std::to_address(std::ranges::find_if(span, std::forward<decltype(predicate)>(predicate))),
                             std::to_address(span.end())};
        }

        [[nodiscard]] constexpr auto TakeBack(size_t n) const noexcept -> ArraySpan
        {
            return ArraySpan{span.subspan(span.size() - n)};
        }
        [[nodiscard]] constexpr auto DropBack(size_t n) const noexcept -> ArraySpan
        {
            return ArraySpan{span.subspan(0, span.size() - n)};
        }
        [[nodiscard]] constexpr auto TakeBackUntil(auto&& predicate) const noexcept -> ArraySpan
        {
            return ArraySpan{std::to_address(std::ranges::find_if(span | std::views::reverse,
                                                                  std::forward<decltype(predicate)>(predicate))),
                             std::to_address(span.end())};
        }
        [[nodiscard]] constexpr auto DropBackUntil(auto&& predicate) const noexcept -> ArraySpan
        {
            return ArraySpan{std::to_address(span.begin()),
                             std::to_address(std::ranges::find_if(span | std::views::reverse,
                                                                  std::forward<decltype(predicate)>(predicate)))};
        }

        [[nodiscard]] constexpr auto StdSpan() const noexcept -> SpanType
        {
            return span;
        }

        // Iterator

        [[nodiscard]] constexpr auto data() const noexcept -> T*
        {
            return span.data();
        }

        [[nodiscard]] constexpr auto begin() noexcept -> Iterator
        {
            return span.begin();
        }
        [[nodiscard]] constexpr auto begin() const noexcept -> ConstIterator
        {
            return span.begin();
        }
        [[nodiscard]] constexpr auto end() noexcept -> Iterator
        {
            return span.end();
        }
        [[nodiscard]] constexpr auto end() const noexcept -> ConstIterator
        {
            return span.end();
        }

        constexpr auto operator[](size_t index) const noexcept -> T&
        {
            return span[index];
        }

        [[nodiscard]] friend constexpr auto operator==(ArraySpan lhs, ArraySpan rhs) noexcept -> bool
        {
            return std::ranges::equal(lhs.span, rhs.span);
        }
        [[nodiscard]] friend constexpr auto operator<=>(ArraySpan lhs, ArraySpan rhs) noexcept -> std::strong_ordering
        {
            return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
    };

    template <typename T, size_t Extent = std::dynamic_extent>
    using ArrayView = ArraySpan<const T, Extent>;

} // namespace glsld