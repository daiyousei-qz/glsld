#pragma once
#include "Basic/StringView.h"

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <type_traits>

namespace glsld
{
    template <typename E>
        requires std::is_enum_v<E>
    auto EnumToString(E value) -> StringView
    {
        return magic_enum::enum_name(value);
    }

    template <typename E, std::unsigned_integral T = uint32_t>
        requires std::is_enum_v<E>
    class EnumBitFlags
    {
    private:
        static consteval auto ValidateEnum() -> bool
        {
            using EnumUnderlyingType = std::underlying_type_t<E>;

            for (auto value : magic_enum::enum_values<E>()) {
                if (std::is_signed_v<EnumUnderlyingType> && static_cast<EnumUnderlyingType>(value) < 0) {
                    return false;
                }
                if (static_cast<EnumUnderlyingType>(value) >= static_cast<EnumUnderlyingType>(sizeof(T) * 8)) {
                    return false;
                }
            }
            return true;
        }

        static_assert(ValidateEnum(),
                      "All enum values of E must be non-negative and not exceeding the number of bits in T.");

        T bits = 0;

        constexpr EnumBitFlags(T bits) : bits(bits)
        {
        }

    public:
        constexpr EnumBitFlags() = default;
        constexpr EnumBitFlags(E flag)
        {
            SetBit(flag);
        }
        constexpr EnumBitFlags(std::initializer_list<E> flags)
        {
            for (auto flag : flags) {
                SetBit(flag);
            }
        }

        constexpr auto GetBits() const noexcept -> T
        {
            return bits;
        }

        constexpr auto TestBit(E flag) const noexcept -> bool
        {
            T flagBit = 1 << static_cast<T>(flag);
            return (bits & flagBit) != 0;
        }

        constexpr auto SetBit(E flag) noexcept -> void
        {
            bits |= 1 << static_cast<uint32_t>(flag);
        }

        constexpr auto ClearBit(E flag) noexcept -> void
        {
            bits &= ~(1 << static_cast<uint32_t>(flag));
        }

        constexpr auto ClearAll() noexcept -> void
        {
            bits = 0;
        }

        constexpr auto operator&(EnumBitFlags<E, T> rhs) const noexcept -> EnumBitFlags<E, T>
        {
            return EnumBitFlags<E, T>(bits & rhs.bits);
        }
        constexpr auto operator&=(EnumBitFlags<E, T> rhs) noexcept -> EnumBitFlags<E, T>&
        {
            bits &= rhs.bits;
            return *this;
        }
        constexpr auto operator|(EnumBitFlags<E, T> rhs) const noexcept -> EnumBitFlags<E, T>
        {
            return EnumBitFlags<E, T>(bits | rhs.bits);
        }
        constexpr auto operator|=(EnumBitFlags<E, T> rhs) noexcept -> EnumBitFlags<E, T>&
        {
            bits |= rhs.bits;
            return *this;
        }
        constexpr auto operator==(const EnumBitFlags<E, T>& rhs) const noexcept -> bool = default;
    };
} // namespace glsld