#pragma once
#include "Common.h"

#include <string>
#include <string_view>

namespace glsld
{
    class MarkdownBuilder
    {
    public:
        MarkdownBuilder() = default;

        auto Append(StringView s) -> void
        {
            buffer += s;
        }

        auto AppendParagraph(StringView s) -> void
        {
            buffer += s;
            buffer += "\n\n";
        }

        auto AppendRuler() -> void
        {
            buffer += "\n---\n";
        }

        auto AppendHeader(StringView s, int level) -> void
        {
            buffer += std::string(level, '#');
            buffer += ' ';
            buffer += s;
            buffer += "\n\n";
        }

        auto AppendCodeBlock(StringView s, StringView language = "") -> void
        {
            buffer += "```";
            buffer += language;
            buffer += "\n";
            buffer += s;
            buffer += "\n```\n\n";
        }

        auto Export() -> std::string
        {
            return std::move(buffer);
        }

    private:
        std::string buffer;
    };
} // namespace glsld