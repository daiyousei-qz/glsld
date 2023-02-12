#pragma once
#include "ranges"
#include "string_view"

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
        constexpr StringView(const char* p) : data(p)
        {
        }
        constexpr StringView(const char* p, size_t count) : data(p, count)
        {
        }
        constexpr StringView(std::nullptr_t p) = delete;
        constexpr StringView(std::string_view s) : data(s)
        {
        }

        explicit StringView(const std::string& s) : data(s)
        {
        }

        template <typename It, typename End>
        constexpr StringView(It first, End last) : data(first, last)
        {
        }

        constexpr StringView(const StringView&) noexcept                    = default;
        constexpr auto operator=(const StringView&) noexcept -> StringView& = default;

        [[nodiscard]] constexpr auto Empty() const noexcept -> bool
        {
            return data.empty();
        }

        [[nodiscard]] constexpr auto Size() const noexcept -> size_t
        {
            return data.size();
        }

        [[nodiscard]] constexpr auto Front() const noexcept -> char
        {
            return data.front();
        }
        [[nodiscard]] constexpr auto Back() const noexcept -> char
        {
            return data.back();
        }

        // Operations

        [[nodiscard]] constexpr auto Split(char seperator) const noexcept -> std::pair<StringView, StringView>
        {
            auto offset = data.find(seperator);
            if (offset != std::string_view::npos) {
                return std::pair{Take(offset), Drop(offset + 1)};
            }
            else {
                return std::pair{*this, StringView{data.end(), data.end()}};
            }
        }
        [[nodiscard]] constexpr auto Split(StringView seperator) const noexcept -> std::pair<StringView, StringView>
        {
            auto offset = data.find(seperator.data);
            if (offset != std::string_view::npos) {
                return std::pair{Take(offset), Drop(offset + seperator.Size())};
            }
            else {
                return std::pair{*this, StringView{data.end(), data.end()}};
            }
        }

        [[nodiscard]] constexpr auto Trim() const noexcept -> StringView
        {
            return TrimFront().TrimBack();
        }
        [[nodiscard]] constexpr auto TrimFront() const noexcept -> StringView
        {
            for (size_t i = 0; i < data.size(); ++i) {
                if (!isspace(data[i])) {
                    return Drop(i);
                }
            }

            return StringView{};
        }
        [[nodiscard]] constexpr auto TrimBack() const noexcept -> StringView
        {
            for (size_t n = data.size(); n > 0; --n) {
                if (!isspace(data[n - 1])) {
                    return Take(n);
                }
            }

            return StringView{};
        }
        [[nodiscard]] constexpr auto Take(size_t n) const noexcept -> StringView
        {
            return StringView{data.substr(0, n)};
        }
        [[nodiscard]] constexpr auto Drop(size_t n) const noexcept -> StringView
        {
            return StringView{data.substr(n)};
        }
        [[nodiscard]] constexpr auto TakeUntil(CharPredicate auto predicate) const noexcept -> StringView
        {
            // FIXME: forward predicate
            return StringView{data.begin(), std::ranges::find_if(data, predicate)};
        }
        [[nodiscard]] constexpr auto DropUntil(CharPredicate auto predicate) const noexcept -> StringView
        {
            // FIXME: forward predicate
            return StringView{std::ranges::find_if(data, predicate), data.end()};
        }

        [[nodiscard]] constexpr auto TakeBack(size_t n) const noexcept -> StringView;
        [[nodiscard]] constexpr auto DropBack(size_t n) const noexcept -> StringView
        {
            auto s = data;
            s.remove_suffix(n);
            return s;
        }
        [[nodiscard]] constexpr auto TakeBackUntil(CharPredicate auto predicate) const noexcept -> StringView;
        [[nodiscard]] constexpr auto DropBackUntil(CharPredicate auto predicate) const noexcept -> StringView;

        [[nodiscard]] constexpr auto SubStr(size_t start, size_t N = npos) const noexcept -> StringView
        {
            return StringView{data.substr(start, N)};
        }

        [[nodiscard]] auto Str() const noexcept -> std::string
        {
            return std::string{data};
        }
        [[nodiscard]] constexpr auto StdStrView() const noexcept -> std::string_view
        {
            return data;
        }

        // Testing

        [[nodiscard]] constexpr auto Contains(StringView s) const noexcept -> bool
        {
            return data.find(s.data) != std::string_view::npos;
        }

        [[nodiscard]] constexpr auto StartWith(StringView s) const noexcept -> bool
        {
            return data.starts_with(s.data);
        }
        [[nodiscard]] constexpr auto EndWith(StringView s) const noexcept -> bool
        {
            return data.ends_with(s.data);
        }

        [[nodiscard]] constexpr auto Data() const noexcept -> const char*
        {
            return data.data();
        }

        [[nodiscard]] constexpr auto begin() noexcept
        {
            return data.begin();
        }
        [[nodiscard]] constexpr auto begin() const noexcept
        {
            return data.begin();
        }
        [[nodiscard]] constexpr auto end() noexcept
        {
            return data.end();
        }
        [[nodiscard]] constexpr auto end() const noexcept
        {
            return data.end();
        }

        constexpr auto operator[](size_t index) const noexcept -> const char&
        {
            return data[index];
        }

        [[nodiscard]] friend constexpr auto operator==(StringView lhs, StringView rhs) noexcept -> bool
        {
            return lhs.data == rhs.data;
        }
        [[nodiscard]] friend constexpr auto operator<=>(StringView lhs, StringView rhs) noexcept -> std::strong_ordering
        {
            return lhs.data <=> rhs.data;
        }

        static constexpr auto npos = std::string_view::npos;

    private:
        std::string_view data;
    };

} // namespace glsld

namespace std
{
    template <>
    struct hash<glsld::StringView>
    {
        [[nodiscard]] auto operator()(const glsld::StringView& key) const noexcept -> size_t
        {
            return hash<std::string_view>{}(key.StdStrView());
        }
    };

    // template <>
    // inline constexpr bool ranges::enable_borrowed_range<glsld::StringView> = true;
    // template <>
    // inline constexpr bool ranges::enable_view<glsld::StringView> = true;
} // namespace std

template <>
struct fmt::formatter<glsld::StringView>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return fmt::formatter<std::string_view>{}.parse(ctx);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const glsld::StringView& s, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::formatter<std::string_view>{}.format(s.StdStrView(), ctx);
    }
};