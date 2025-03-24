#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

#include <string>

namespace glsld
{
    class MarkdownBuilder
    {
    private:
        fmt::memory_buffer buffer;

        auto Append(StringView s) -> void
        {
            buffer.append(s.data(), s.data() + s.Size());
        }

    public:
        MarkdownBuilder() = default;

        auto AppendRuler() -> void
        {
            Append("\n---\n");
        }

        auto AppendParagraph(StringView s) -> void
        {
            Append(s);
            Append("\n\n");
        }

        template <typename... Args>
        auto AppendParagraph(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
            Append("\n\n");
        }

        auto AppendBullet(StringView s) -> void
        {
            Append("- ");
            Append(s);
            Append("\n");
        }

        template <typename... Args>
        auto AppendBullet(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            Append("- ");
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
            Append("\n");
        }

        auto AppendHeader(int level, StringView s) -> void
        {
            GLSLD_ASSERT(level > 0);
            Append(std::string(level, '#'));
            Append(" ");
            Append(s);
            Append("\n\n");
        }

        template <typename... Args>
        auto AppendHeader(int level, fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            GLSLD_ASSERT(level > 0);
            Append(std::string(level, '#'));
            Append(" ");
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
            Append("\n\n");
        }

        auto AppendCodeBlock(StringView s, StringView language = "") -> void
        {
            Append("```");
            Append(language);
            Append("\n");
            Append(s);
            Append("\n```\n\n");
        }

        auto Export() -> std::string
        {
            return std::string(buffer.data(), buffer.size());
        }
    };
} // namespace glsld