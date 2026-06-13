#pragma once
#include "Support/EnumReflection.h"
#include "Support/StringView.h"
#include "Basic/SourceInfo.h"

#include <catch2/catch_tostring.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <expected>

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

    template <>
    struct StringMaker<glsld::TextPosition>
    {
        static auto convert(const glsld::TextPosition& value) -> std::string
        {
            return fmt::format("TextPosition(line:{}, character:{})", value.line + 1, value.character + 1);
        }
    };

    template <>
    struct StringMaker<glsld::TextRange>
    {
        static auto convert(const glsld::TextRange& value) -> std::string
        {
            return fmt::format("TextRange(lineBegin:{}, characterBegin:{}, lineEnd:{}, characterEnd:{})",
                               value.start.line + 1, value.start.character + 1, value.end.line + 1,
                               value.end.character + 1);
        }
    };

    template <typename E>
        requires std::is_enum_v<E>
    struct StringMaker<E>
    {
        static auto convert(E value) -> std::string
        {
            return glsld::EnumToString(value).Str();
        }
    };

    template <typename T>
    struct StringMaker<std::unexpected<T>>
    {
        static auto convert(const std::unexpected<T>& value) -> std::string
        {
            return fmt::format("std::unexpected({})", Catch::StringMaker<T>::convert(value.error()));
        }
    };

    template <typename T, typename E>
    struct StringMaker<std::expected<T, E>>
    {
        static auto convert(const std::expected<T, E>& value) -> std::string
        {
            if (value.has_value()) {
                if constexpr (std::is_same_v<T, void>) {
                    return "std::expected()";
                }
                else {
                    return fmt::format("std::expected({})", Catch::StringMaker<T>::convert(value.value()));
                }
            }
            else {
                return fmt::format("std::unexpected({})", Catch::StringMaker<E>::convert(value.error()));
            }
        }
    };
} // namespace Catch