#pragma once
#include "Support/StringView.h"

#include <expected>
#include <memory>

namespace glsld
{
    enum class TextTransportError
    {
        Unknown,
    };

    class TextTransport
    {
    public:
        TextTransport()          = default;
        virtual ~TextTransport() = default;

        // Returns a line of text that should live until the next call to ReadLine/Read.
        virtual auto ReadLine() -> std::expected<StringView, TextTransportError> = 0;

        // Returns a block of text that should live until the next call to ReadLine/Read.
        virtual auto Read(size_t size) -> std::expected<StringView, TextTransportError> = 0;

        // Writes a block of text. Returns true if the write is successful.
        virtual auto Write(StringView data) -> bool = 0;

        // Flushes any buffered data. Returns true if the flush is successful.
        virtual auto Flush() -> bool = 0;
    };

    auto CreateStdioTextTransport() -> std::unique_ptr<TextTransport>;

} // namespace glsld