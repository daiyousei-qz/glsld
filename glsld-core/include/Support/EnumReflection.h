#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

#include <magic_enum/magic_enum.hpp>

#include <type_traits>

namespace glsld
{
    template <typename E>
        requires std::is_enum_v<E>
    inline auto EnumToString(E value) -> StringView
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

        static constexpr auto GetEnumValues() -> ArrayView<E, magic_enum::enum_count<E>()>
        {
            constexpr static auto values = magic_enum::enum_values<E>();
            return values;
        }

        class EnumFlagSentinel
        {
        };

        class EnumFlagIterator
        {
        private:
            EnumBitFlags<E, T> bits;
            size_t index;

            friend class EnumBitFlags<E, T>;

            // Moves the iterator to the next set bit if the current bit is not set
            constexpr auto MoveToSetBit() -> void
            {
                while (index < GetEnumValues().size() && !bits.TestBit(GetEnumValues()[index])) {
                    ++index;
                }
            }

            constexpr EnumFlagIterator(EnumBitFlags<E, T> bits, size_t index) : bits(bits), index(index)
            {
            }

        public:
            constexpr auto operator*() const -> E
            {
                return GetEnumValues()[index];
            }

            constexpr auto operator++() -> EnumFlagIterator&
            {
                ++index;
                MoveToSetBit();
                return *this;
            }

            constexpr auto operator==(EnumFlagSentinel) const -> bool
            {
                return index >= GetEnumValues().size();
            }
        };

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
            T flagBit = static_cast<T>(1) << static_cast<T>(flag);
            return (bits & flagBit) != 0;
        }

        constexpr auto SetBit(E flag) noexcept -> void
        {
            bits |= static_cast<T>(1) << static_cast<T>(flag);
        }

        constexpr auto ClearBit(E flag) noexcept -> void
        {
            bits &= ~(static_cast<T>(1) << static_cast<T>(flag));
        }

        constexpr auto ClearAll() noexcept -> void
        {
            bits = 0;
        }

        constexpr auto begin() const noexcept -> EnumFlagIterator
        {
            auto result = EnumFlagIterator{*this, 0};
            result.MoveToSetBit();
            return result;
        }
        constexpr auto end() const noexcept -> EnumFlagSentinel
        {
            return EnumFlagSentinel{};
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