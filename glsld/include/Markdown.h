#pragma once
#include <string>
#include <string_view>

namespace glsld
{
    class MarkdownBuilder
    {
    public:
        auto Append(std::string_view s)
        {
            buffer += s;
        }

        auto AppendParagraph(std::string_view s)
        {
            buffer += s;
            buffer += "\n\n";
        }

        auto AppendRuler()
        {
            buffer += "\n---\n";
        }

        auto Export() -> std::string
        {
            return std::move(buffer);
        }

    private:
        std::string buffer;
    };
} // namespace glsld