#pragma once
#include <cassert>
#include <span>

// FIXME: improve the assert message
#define GLSLD_ASSERT(EXPR) assert(EXPR)
#define GLSLD_REQUIRE(EXPR) static_cast<void>((!!(EXPR)) || (std::abort(), false))

#define GLSLD_UNREACHABLE() (GLSLD_ASSERT(false), std::abort())
#define GLSLD_NO_IMPL() (GLSLD_ASSERT(false), std::abort())

namespace glsld
{
    template <typename T>
    inline constexpr bool AlwaysFalse = false;

    template <typename T>
    concept Hashable = requires(T value) {
        { value.GetHashCode() } -> std::convertible_to<size_t>;
    };

    template <typename T, size_t Extent = std::dynamic_extent>
    using ArraySpan = std::span<T, Extent>;

    template <typename T, size_t Extent = std::dynamic_extent>
    using ArrayView = std::span<const T, Extent>;

    template <typename T>
        requires std::is_pointer_v<T>
    struct NotNull
    {
        using PointerType = T;
        using ElementType = std::remove_pointer_t<T>;

        PointerType ptr;

        NotNull() = delete;
        NotNull(PointerType ptr) : ptr(ptr)
        {
            GLSLD_ASSERT(ptr != nullptr);
        }

        operator ElementType*() const noexcept
        {
            return ptr;
        }

        auto operator->() const noexcept -> PointerType
        {
            return ptr;
        }

        auto GetPointer() const noexcept -> PointerType
        {
            return ptr;
        }
    };
} // namespace glsld

template <glsld::Hashable T>
struct std::hash<T>
{
    auto operator()(const T& value) const -> size_t
    {
        return value.GetHashCode();
    }
};