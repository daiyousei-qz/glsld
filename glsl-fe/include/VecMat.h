#pragma once
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
        {
            return Vec{-data};
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
        auto operator%=(const Vec& other) noexcept -> Vec&
            requires(std::is_integral_v<T>)
        {
            data %= other.data;
        }

        auto operator&=(const Vec& other) noexcept -> Vec&
            requires(std::is_integral_v<T>)
        {
            data &= other.data;
        }
        auto operator|=(const Vec& other) noexcept -> Vec&
            requires(std::is_integral_v<T>)
        {
            data |= other.data;
        }
        auto operator^=(const Vec& other) noexcept -> Vec&
            requires(std::is_integral_v<T>)
        {
            data ^= other.data;
        }

        auto operator<<=(const Vec& other) noexcept -> Vec&
            requires(std::is_integral_v<T>)
        {
            data <<= other.data;
        }
        auto operator>>=(const Vec& other) noexcept -> Vec&
            requires(std::is_integral_v<T>)
        {
            data >>= other.data;
        }

        auto operator+(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result += other;
            return result;
        }
        auto operator-(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result += other;
            return result;
        }
        auto operator*(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result += other;
            return result;
        }
        auto operator/(const Vec& other) const noexcept -> Vec
        {
            Vec result = *this;
            result += other;
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
    private:
        std::array<T, M * N> data;
    };

} // namespace glsld