#pragma once
#include "SourceView.h"

namespace glsld
{
    // A scanner of some source code string, providing utilities to lex the source text.
    // NOTE srcEnd must be a '\0' character
    class SourceScanner
    {
    public:
        SourceScanner() = default;
        SourceScanner(const char* srcBegin, const char* srcEnd)
            : srcBegin(srcBegin), srcEnd(srcEnd), srcCursor(srcBegin)
        {
            GLSLD_REQUIRE(*srcEnd == '\0');
        }

        // Returns true if the cursor is at the beginning of the source.
        auto CursorAtBegin() const noexcept -> bool
        {
            return srcCursor == srcBegin;
        }

        // Returns true if the cursor is at the end of the source.
        auto CursorAtEnd() const noexcept -> bool
        {
            return srcCursor == srcEnd;
        }

        auto GetTextPosition() -> TextPosition
        {
            return TextPosition{lineIndex, columnIndex};
        };

        // Skip whitespace and return true if a '\n' is encountered
        auto SkipWhitespace(bool& skippedWhitespace, bool& skippedNewline) -> void
        {
            while (!CursorAtEnd()) {
                auto ch = PeekChar();

                if (ch == ' ' || ch == '\t' || ch == '\r') {
                    ++columnIndex;

                    ++srcCursor;
                }
                else if (ch == '\n') {
                    ++lineIndex;
                    columnIndex = 0;

                    skippedNewline = true;
                    ++srcCursor;
                }
                else {
                    break;
                }

                skippedWhitespace = true;
            }
        }

        auto SkipChar(size_t num) -> void
        {
            // FIXME: optimize this
            for (int i = 0; i < num; ++i) {
                ConsumeChar();
            }
        }

        auto ConsumeChar() -> char
        {
            if (CursorAtEnd()) {
                return '\0';
            }

            auto result = *srcCursor++;

            // Update line and column index
            if (result == '\n') {
                ++lineIndex;
                columnIndex = 0;
            }
            else {
                ++columnIndex;
            }

            // Skip line continuation
            if (PeekChar() == '\\') {
                if (PeekChar(1) == '\n') {
                    srcCursor += 2;
                    ++lineIndex;
                    columnIndex = 0;
                }
                else if (PeekChar(1) == '\r' && PeekChar(2) == '\n') {
                    srcCursor += 3;
                    ++lineIndex;
                    columnIndex = 0;
                }
            }

            return result;
        }

        auto PeekChar() -> char
        {
            return *srcCursor;
        }

        auto PeekChar(size_t lookahead) -> char
        {
            if (lookahead > srcEnd - srcCursor) {
                return '\0';
            }

            return *(srcCursor + lookahead);
        }

        auto Clone() -> SourceScanner
        {
            return *this;
        }

        auto Restore(SourceScanner srcView) -> void
        {
            *this = srcView;
        }

    private:
        // The beginning of the source string
        const char* srcBegin = nullptr;

        // The end of the source string, which is always a null terminator
        const char* srcEnd = nullptr;

        // The current cursor position in the source string
        const char* srcCursor = nullptr;

        int lineIndex   = 0;
        int columnIndex = 0;
    };
} // namespace glsld