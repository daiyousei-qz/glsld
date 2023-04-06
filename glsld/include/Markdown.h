#pragma once
#include <string>
#include <string_view>

namespace glsld
{
    class MarkdownBuilder
    {
    public:
        MarkdownBuilder() = default;

        auto Append(std::string_view s) -> void
        {
            buffer += s;
        }

        auto AppendParagraph(std::string_view s) -> void
        {
            buffer += s;
            buffer += "\n\n";
        }

        auto AppendRuler() -> void
        {
            buffer += "\n---\n";
        }

        auto AppendHeader(std::string_view s, int level) -> void
        {
            buffer += std::string(level, '#');
            buffer += ' ';
            buffer += s;
            buffer += "\n\n";
        }

        auto AppendCodeBlock(std::string_view s) -> void
        {
            buffer += "```\n";
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