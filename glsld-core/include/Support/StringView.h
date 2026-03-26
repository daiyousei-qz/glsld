#pragma once
#include <fmt/format.h>

#include <concepts>
#include <ranges>
#include <string_view>

namespace glsld
{
    // This class wraps around std::string_view and adds string utilities
    class StringView : std::ranges::view_base
    {
    private:
        std::string_view view;

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
        constexpr StringView(std::string_view s) : view(s)
        {
        }
        constexpr explicit StringView(const std::string& s) : view(s)
        {
        }
        constexpr StringView(std::nullptr_t p) = delete;

        template <size_t N>
            requires(N > 0)
        constexpr StringView(const char (&s)[N]) : view(s, N - 1)
        {
        }

        template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
            requires(!std::is_convertible_v<End, size_t>)
        constexpr StringView(It first, End last) : view(first, last)
        {
            // We let the underlying std::string_view constructor validate the range
        }

        template <std::ranges::contiguous_range R>
        constexpr StringView(R&& r) : view(std::ranges::data(r), std::ranges::size(r))
        {
            // We let the underlying std::string_view constructor validate the range
        }

        constexpr StringView(const StringView&) noexcept                    = default;
        constexpr auto operator=(const StringView&) noexcept -> StringView& = default;

        [[nodiscard]] constexpr auto empty() const noexcept -> bool
        {
            return view.empty();
        }

        [[nodiscard]] constexpr auto size() const noexcept -> size_t
        {
            return view.size();
        }

        [[nodiscard]] constexpr auto front() const noexcept -> char
        {
            return view.front();
        }
        [[nodiscard]] constexpr auto back() const noexcept -> char
        {
            return view.back();
        }

        // Operations

        [[nodiscard]] constexpr auto Split(char separator) const noexcept -> std::pair<StringView, StringView>
        {
            auto offset = view.find(separator);
            if (offset != std::string_view::npos) {
                return std::pair{Take(offset), Drop(offset + 1)};
            }
            else {
                return std::pair{*this, StringView{view.end(), view.end()}};
            }
        }
        [[nodiscard]] constexpr auto Split(StringView separator) const noexcept -> std::pair<StringView, StringView>
        {
            auto offset = view.find(separator.view);
            if (offset != std::string_view::npos) {
                return std::pair{Take(offset), Drop(offset + separator.size())};
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
        template <typename Fn>
            requires std::predicate<Fn, char>
        [[nodiscard]] constexpr auto TakeWhile(Fn predicate) const noexcept -> StringView
        {
            const char* newEnd = view.data();
            while (newEnd != view.data() + view.size() && predicate(*newEnd)) {
                ++newEnd;
            }

            return StringView{view.data(), static_cast<size_t>(newEnd - view.data())};
        }
        template <typename Fn>
            requires std::predicate<Fn, char>
        [[nodiscard]] constexpr auto DropWhile(Fn predicate) const noexcept -> StringView
        {
            const char* newBegin = view.data();
            while (newBegin != view.data() + view.size() && predicate(*newBegin)) {
                ++newBegin;
            }

            return StringView{newBegin, static_cast<size_t>(view.data() + view.size() - newBegin)};
        }

        [[nodiscard]] constexpr auto TakeBack(size_t n) const noexcept -> StringView
        {
            return StringView{view.substr(view.size() - n)};
        }
        [[nodiscard]] constexpr auto DropBack(size_t n) const noexcept -> StringView
        {
            auto s = view;
            s.remove_suffix(n);
            return s;
        }
        template <typename Fn>
            requires std::predicate<Fn, char>
        [[nodiscard]] constexpr auto TakeBackWhile(Fn predicate) const noexcept -> StringView
        {
            const char* newBegin = view.data() + view.size();
            while (newBegin != view.data() && predicate(*(newBegin - 1))) {
                --newBegin;
            }

            return StringView{newBegin, static_cast<size_t>(view.data() + view.size() - newBegin)};
        }
        template <typename Fn>
            requires std::predicate<Fn, char>
        [[nodiscard]] constexpr auto DropBackWhile(Fn predicate) const noexcept -> StringView
        {
            const char* newEnd = view.data() + view.size();
            while (newEnd != view.data() && predicate(*(newEnd - 1))) {
                --newEnd;
            }

            return StringView{view.data(), static_cast<size_t>(newEnd - view.data())};
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