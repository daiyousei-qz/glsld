#pragma once
#include "Basic/SourceInfo.h"
#include "Server/Protocol.h"

namespace glsld
{
    inline auto FromLspPosition(lsp::Position position) -> TextPosition
    {
        return TextPosition{
            .line      = static_cast<int>(position.line),
            .character = static_cast<int>(position.character),
        };
    }
    inline auto ToLspPosition(TextPosition position) -> lsp::Position
    {
        return lsp::Position{
            .line      = static_cast<uint32_t>(position.line),
            .character = static_cast<uint32_t>(position.character),
        };
    }

    inline auto FromLspRange(lsp::Range range) -> TextRange
    {
        return TextRange{
            FromLspPosition(range.start),
            FromLspPosition(range.end),
        };
    }
    inline auto ToLspRange(TextRange range) -> lsp::Range
    {
        return lsp::Range{
            .start = ToLspPosition(range.start),
            .end   = ToLspPosition(range.end),
        };
    }

    inline auto ApplySourceChange(std::string& sourceBuffer, TextRange range, StringView changedText) -> void
    {
        TextPosition cur = {};
        size_t index     = 0;

        // Find the begining index of the range
        for (; index < sourceBuffer.size(); ++index) {
            if (cur >= range.start) {
                break;
            }

            if (sourceBuffer[index] == '\n') {
                cur.line += 1;
                cur.character = 0;
            }
            else {
                cur.character += 1;
            }
        }

        size_t indexBegin = index;

        // Find the ending index of the range
        for (; index < sourceBuffer.size(); ++index) {
            if (cur >= range.end) {
                break;
            }

            if (sourceBuffer[index] == '\n') {
                cur.line += 1;
                cur.character = 0;
            }
            else {
                cur.character += 1;
            }
        }

        size_t indexEnd = index;

        sourceBuffer.replace(indexBegin, indexEnd - indexBegin, changedText.StdStrView());
    }
} // namespace glsld