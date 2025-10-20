#pragma once
#include "Basic/StringView.h"

#include "catch2/catch_tostring.hpp"

namespace Catch
{
    template <>
    struct StringMaker<glsld::StringView>
    {
        static auto convert(glsld::StringView value) -> std::string
        {
            return value.Str();
        }
    };
} // namespace Catch