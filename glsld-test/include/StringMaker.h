#pragma once
#include "Basic/StringView.h"
#include "Basic/SourceInfo.h"
#include "Ast/Base.h"

#include "Support/EnumReflection.h"
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

    template <>
    struct StringMaker<glsld::AstNodeTag>
    {
        static auto convert(const glsld::AstNodeTag& value) -> std::string
        {
            return glsld::EnumToString(value).Str();
        }
    };
} // namespace Catch