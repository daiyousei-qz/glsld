#pragma once
#include "Basic/Print.h"

#include <array>
#include <type_traits>

namespace glsld
{
    template <typename T, size_t N>
    class Vec
    {
    public:
        Vec() = default;
        Vec(std::array<T, N> data) : data(data)
        {
        }

        auto operator+() const noexcept -> Vec
        {
            return Vec{data};
        }
        auto operator-() const noexcept -> Vec
            requires(!std::is_same_v<T, bool>)
        {
            Vec result;
            for (size_t i = 0; i < N; ++i) {
                result.data[i] = -data[i];
            }
            return result;
        }

        auto operator+=(const Vec& other) noexcept -> Vec&
        {
            data += other.data;
        }
        auto operator-=(const Vec& other) noexcept -> Vec&
        {
            data -= other.data;
        }
        auto operator*=(const Vec& other) noexcept -> Vec&
        {
            data *= other.data;
        }
        auto operator/=(const Vec& other) noexcept -> Vec&
        {
            data /= other.data;
        }
        auto operator%=(const Vec& other) noexcept -> Vec& requires(std::is_integral_v<T>) { data %= other.data; }

        auto operator&=(const Vec& other) noexcept
            -> Vec& requires(std::is_integral_v<T>) { data &= other.data; } auto operator|=(const Vec& other) noexcept
            -> Vec& requires(std::is_integral_v<T>) { data |= other.data; } auto operator^=(const Vec& other) noexcept
            -> Vec& requires(std::is_integral_v<T>) { data ^= other.data; }

        auto operator<<=(const Vec& other) noexcept
            -> Vec& requires(std::is_integral_v<T>) { data <<= other.data; } auto operator>>=(const Vec& other) noexcept
            -> Vec& requires(std::is_integral_v<T>) { data >>= other.data; }

        auto operator+(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result += other;
            return result;
        }
        auto operator-(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result -= other;
            return result;
        }
        auto operator*(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result *= other;
            return result;
        }
        auto operator/(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result /= other;
            return result;
        }
        auto operator%(const Vec& other) const noexcept -> Vec
            requires(std::is_integral_v<T>)
        {
            Vec result = *this;
            result %= other;
            return result;
        }

        auto operator&(const Vec& other) const noexcept -> Vec
            requires(std::is_integral_v<T>)
        {
            Vec result = *this;
            result &= other;
            return result;
        }
        auto operator|(const Vec& other) const noexcept -> Vec
            requires(std::is_integral_v<T>)
        {
            Vec result = *this;
            result |= other;
            return result;
        }
        auto operator^(const Vec& other) const noexcept -> Vec
            requires(std::is_integral_v<T>)
        {
            Vec result = *this;
            result ^= other;
            return result;
        }

        auto operator<<(const Vec& other) const noexcept -> Vec
            requires(std::is_integral_v<T>)
        {
            Vec result = *this;
            result <<= other;
            return result;
        }
        auto operator>>(const Vec& other) const noexcept -> Vec
            requires(std::is_integral_v<T>)
        {
            Vec result = *this;
            result >>= other;
            return result;
        }

        auto operator==(const Vec& other) const noexcept -> bool = default;

    private:
        std::array<T, N> data;
    };

    template <typename T, size_t M, size_t N>
    class Mat
    {
    public:
        Mat() = default;
        Mat(std::array<T, M * N> data) : data(data)
        {
        }

        auto operator+() const noexcept -> Mat
        {
            return Mat{data};
        }
        auto operator-() const noexcept -> Mat
        {
            Mat result;
            for (size_t i = 0; i < M * N; ++i) {
                result.data[i] = -data[i];
            }
            return result;
        }

        auto operator+=(const Mat& other) noexcept -> Mat&
        {
            data += other.data;
        }
        auto operator-=(const Mat& other) noexcept -> Mat&
        {
            data -= other.data;
        }
        auto operator*=(const Mat& other) noexcept -> Mat&
        {
            data *= other.data;
        }
        auto operator/=(const Mat& other) noexcept -> Mat&
        {
            data /= other.data;
        }
        auto operator%=(const Mat& other) noexcept -> Mat& requires(std::is_integral_v<T>) { data %= other.data; }

        auto operator+(const Mat& other) const noexcept -> Mat
        {
            Mat result = *this;
            result += other;
            return result;
        }
        auto operator-(const Mat& other) const noexcept -> Mat
        {
            Mat result = *this;
            result -= other;
            return result;
        }
        auto operator*(const Mat& other) const noexcept -> Mat
        {
            Mat result = *this;
            result *= other;
            return result;
        }
        auto operator/(const Mat& other) const noexcept -> Mat
        {
            Mat result = *this;
            result /= other;
            return result;
        }
        auto operator%(const Mat& other) const noexcept -> Mat
            requires(std::is_integral_v<T>)
        {
            Mat result = *this;
            result %= other;
            return result;
        }

        auto operator==(const Mat& other) const noexcept -> bool                  = default;
        auto operator<=>(const Mat& other) const noexcept -> std::strong_ordering = delete;

    private:
        std::array<T, M * N> data;
    };

    using Vec2b = Vec<bool, 2>;
    using Vec3b = Vec<bool, 3>;
    using Vec4b = Vec<bool, 4>;
    using Vec2f = Vec<float, 2>;
    using Vec3f = Vec<float, 3>;
    using Vec4f = Vec<float, 4>;
    using Vec2d = Vec<double, 2>;
    using Vec3d = Vec<double, 3>;
    using Vec4d = Vec<double, 4>;
    using Vec2i = Vec<int32_t, 2>;
    using Vec3i = Vec<int32_t, 3>;
    using Vec4i = Vec<int32_t, 4>;
    using Vec2u = Vec<uint32_t, 2>;
    using Vec3u = Vec<uint32_t, 3>;
    using Vec4u = Vec<uint32_t, 4>;

    using Mat2x2f = Mat<float, 2, 2>;
    using Mat2x3f = Mat<float, 2, 3>;
    using Mat2x4f = Mat<float, 2, 4>;
    using Mat3x2f = Mat<float, 3, 2>;
    using Mat3x3f = Mat<float, 3, 3>;
    using Mat3x4f = Mat<float, 3, 4>;
    using Mat4x2f = Mat<float, 4, 2>;
    using Mat4x3f = Mat<float, 4, 3>;
    using Mat4x4f = Mat<float, 4, 4>;
    using Mat2f   = Mat<float, 2, 2>;
    using Mat3f   = Mat<float, 3, 3>;
    using Mat4f   = Mat<float, 4, 4>;

    using Mat2x2d = Mat<double, 2, 2>;
    using Mat2x3d = Mat<double, 2, 3>;
    using Mat2x4d = Mat<double, 2, 4>;
    using Mat3x2d = Mat<double, 3, 2>;
    using Mat3x3d = Mat<double, 3, 3>;
    using Mat3x4d = Mat<double, 3, 4>;
    using Mat4x2d = Mat<double, 4, 2>;
    using Mat4x3d = Mat<double, 4, 3>;
    using Mat4x4d = Mat<double, 4, 4>;
    using Mat2d   = Mat<double, 2, 2>;
    using Mat3d   = Mat<double, 3, 3>;
    using Mat4d   = Mat<double, 4, 4>;
} // namespace glsld

template <typename T, size_t N>
struct fmt::formatter<glsld::Vec<T, N>>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return fmt::formatter<std::string_view>{}.parse(ctx);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const glsld::Vec<T, N>& v, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::formatter<std::string_view>{}.format("vec", ctx);
    }
};

template <typename T, size_t M, size_t N>
struct fmt::formatter<glsld::Mat<T, M, N>>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return fmt::formatter<std::string_view>{}.parse(ctx);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const glsld::Mat<T, M, N>& m, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::formatter<std::string_view>{}.format("mat", ctx);
    }
};