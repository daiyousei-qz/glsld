#pragma once
#include <type_traits>

namespace glsld
{
    template <typename T>
        requires std::is_enum_v<T>
    struct StringEnum
    {
        T value;

        StringEnum() = default;
        StringEnum(T value) : value(value)
        {
        }

        operator T() const
        {
            return value;
        }
    };
} // namespace glsld