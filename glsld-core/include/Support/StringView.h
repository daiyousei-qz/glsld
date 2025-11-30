#pragma once
#include <concepts>
#include <ranges>
#include <string_view>

#include "fmt/format.h"

namespace glsld
{
    template <typename F>
    concept CharPredicate = std::predicate<F, char>;

    // This class wraps around std::string_view and adds string utilities
    class StringView : std::ranges::view_base
    {
    public:
        using Iterator      = std::string_view::iterator;
        using ConstIterator = std::string_view::const_iterator;

        constexpr StringView() = default;
        constexpr StringView(const char* p) : view(p)
        {
        }
        constexpr StringView(const char* p, size_t count) : view(p, count)
        {
        }
        template <size_t N>
        constexpr StringView(const char (&s)[N]) : view(s, N - 1)
        {
        }
        constexpr StringView(std::nullptr_t p) = delete;
        constexpr StringView(std::string_view s) : view(s)
        {
        }

        explicit StringView(const std::string& s) : view(s)
        {
        }

        template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
            requires std::is_same_v<std::iter_value_t<It>, char> && (!std::is_convertible_v<End, size_t>)
        constexpr StringView(It first, End last) : view(first, last)
        {
        }

        template <std::ranges::contiguous_range R>
            requires std::is_same_v<std::ranges::range_value_t<R>, char>
        constexpr StringView(R&& r) : view(std::ranges::data(r), std::ranges::size(r))
        {
        }

        constexpr StringView(const StringView&) noexcept                    = default;
        constexpr auto operator=(const StringView&) noexcept -> StringView& = default;

        [[nodiscard]] constexpr auto Empty() const noexcept -> bool
        {
            return view.empty();
        }

        [[nodiscard]] constexpr auto Size() const noexcept -> size_t
        {
            return view.size();
        }

        [[nodiscard]] constexpr auto Front() const noexcept -> char
        {
            return view.front();
        }
        [[nodiscard]] constexpr auto Back() const noexcept -> char
        {
            return view.back();
        }

        // Operations

        [[nodiscard]] constexpr auto Split(char seperator) const noexcept -> std::pair<StringView, StringView>
        {
            auto offset = view.find(seperator);
            if (offset != std::string_view::npos) {
                return std::pair{Take(offset), Drop(offset + 1)};
            }
            else {
                return std::pair{*this, StringView{view.end(), view.end()}};
            }
        }
        [[nodiscard]] constexpr auto Split(StringView seperator) const noexcept -> std::pair<StringView, StringView>
        {
            auto offset = view.find(seperator.view);
            if (offset != std::string_view::npos) {
                return std::pair{Take(offset), Drop(offset + seperator.Size())};
            }
            else {
                return std::pair{*this, StringView{view.end(), view.end()}};
            }
        }

        [[nodiscard]] constexpr auto Trim() const noexcept -> StringView
        {
            return TrimFront().TrimBack();
        }
        [[nodiscard]] constexpr auto TrimFront() const noexcept -> StringView
        {
            for (size_t i = 0; i < view.size(); ++i) {
                if (!isspace(view[i])) {
                    return Drop(i);
                }
            }

            return StringView{};
        }
        [[nodiscard]] constexpr auto TrimBack() const noexcept -> StringView
        {
            for (size_t n = view.size(); n > 0; --n) {
                if (!isspace(view[n - 1])) {
                    return Take(n);
                }
            }

            return StringView{};
        }
        [[nodiscard]] constexpr auto Take(size_t n) const noexcept -> StringView
        {
            return StringView{view.substr(0, n)};
        }
        [[nodiscard]] constexpr auto Drop(size_t n) const noexcept -> StringView
        {
            return StringView{view.substr(n)};
        }
        [[nodiscard]] constexpr auto TakeUntil(CharPredicate auto&& predicate) const noexcept -> StringView
        {
            return StringView{view.begin(), std::ranges::find_if(view, std::forward<decltype(predicate)>(predicate))};
        }
        [[nodiscard]] constexpr auto DropUntil(CharPredicate auto&& predicate) const noexcept -> StringView
        {
            return StringView{std::ranges::find_if(view, std::forward<decltype(predicate)>(predicate)), view.end()};
        }

        [[nodiscard]] constexpr auto TakeBack(size_t n) const noexcept -> StringView;
        [[nodiscard]] constexpr auto DropBack(size_t n) const noexcept -> StringView
        {
            auto s = view;
            s.remove_suffix(n);
            return s;
        }
        [[nodiscard]] constexpr auto TakeBackUntil(CharPredicate auto predicate) const noexcept -> StringView;
        [[nodiscard]] constexpr auto DropBackUntil(CharPredicate auto predicate) const noexcept -> StringView;

        [[nodiscard]] constexpr auto SubStr(size_t start, size_t N = npos) const noexcept -> StringView
        {
            return StringView{view.substr(start, N)};
        }

        [[nodiscard]] auto Str() const noexcept -> std::string
        {
            return std::string{view};
        }
        [[nodiscard]] constexpr auto StdStrView() const noexcept -> std::string_view
        {
            return view;
        }

        // Predicates

        [[nodiscard]] constexpr auto Contains(char ch) const noexcept -> bool
        {
            return view.find(ch) != std::string_view::npos;
        }

        [[nodiscard]] constexpr auto Contains(StringView s) const noexcept -> bool
        {
            return view.find(s.view) != std::string_view::npos;
        }

        [[nodiscard]] constexpr auto StartWith(char ch) const noexcept -> bool
        {
            return view.starts_with(ch);
        }
        [[nodiscard]] constexpr auto StartWith(StringView s) const noexcept -> bool
        {
            return view.starts_with(s.view);
        }

        [[nodiscard]] constexpr auto EndWith(char ch) const noexcept -> bool
        {
            return view.ends_with(ch);
        }
        [[nodiscard]] constexpr auto EndWith(StringView s) const noexcept -> bool
        {
            return view.ends_with(s.view);
        }

        [[nodiscard]] constexpr auto data() const noexcept -> const char*
        {
            return view.data();
        }

        [[nodiscard]] constexpr auto begin() noexcept -> Iterator
        {
            return view.begin();
        }
        [[nodiscard]] constexpr auto begin() const noexcept -> ConstIterator
        {
            return view.begin();
        }
        [[nodiscard]] constexpr auto end() noexcept -> Iterator
        {
            return view.end();
        }
        [[nodiscard]] constexpr auto end() const noexcept -> ConstIterator
        {
            return view.end();
        }

        auto GetHashCode() const noexcept -> size_t
        {
            return std::hash<std::string_view>{}(view);
        }

        constexpr explicit operator std::string() const noexcept
        {
            return std::string{view};
        }

        constexpr auto operator[](size_t index) const noexcept -> const char&
        {
            return view[index];
        }

        [[nodiscard]] friend constexpr auto operator==(StringView lhs, StringView rhs) noexcept -> bool
        {
            return lhs.view == rhs.view;
        }
        [[nodiscard]] friend constexpr auto operator<=>(StringView lhs, StringView rhs) noexcept -> std::strong_ordering
        {
            return lhs.view <=> rhs.view;
        }

        static constexpr auto npos = std::string_view::npos;

    private:
        std::string_view view;
    };

    inline auto operator+=(std::string& lhs, StringView rhs) -> std::string&
    {
        return lhs += rhs.StdStrView();
    }

} // namespace glsld

template <>
struct fmt::formatter<glsld::StringView> : private fmt::formatter<std::string_view>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return fmt::formatter<std::string_view>::parse(ctx);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const glsld::StringView& s, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::formatter<std::string_view>::format(s.StdStrView(), ctx);
    }
};