#pragma once
#include "SourceInfo.h"
#include <vector>

namespace glsld
{
    struct ScannerCheckPoint
    {
        const char* srcCursor;
        int lineIndex;
        int characterIndex;
    };

    // FIXME: the source text should have utf-8 encoding
    // FIXME: support utf-16 character offset for vscode
    // A scanner of some source code string, providing utilities to lex the source text.
    // NOTE srcEnd must be a '\0' character
    class SourceScanner
    {
    public:
        SourceScanner() = default;
        SourceScanner(const char* srcBegin, const char* srcEnd, bool countUtf16Characters = false)
            : srcBegin(srcBegin), srcEnd(srcEnd), srcCursor(srcBegin), countUtf16Characters(countUtf16Characters)
        {
            GLSLD_REQUIRE(*srcEnd == '\0');
            // FIXME:
            // TryConsumeLineContinuation();
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

        auto GetTextPosition() const noexcept -> TextPosition
        {
            return TextPosition{lineIndex, characterIndex};
        };

        auto SkipWhitespace(bool& skippedWhitespace, bool& skippedNewline) -> void
        {
            while (!CursorAtEnd()) {
                auto ch = *srcCursor;

                if (ch == ' ' || ch == '\t' || ch == '\r') {
                    ++characterIndex;

                    skippedWhitespace = true;
                    ++srcCursor;
                }
                else if (ch == '\n') {
                    ++lineIndex;
                    characterIndex = 0;

                    skippedWhitespace = true;
                    skippedNewline    = true;
                    ++srcCursor;
                }
                else if (!TryConsumeLineContinuation()) {
                    break;
                }
            }
        }

        // Peek the next code unit.
        auto PeekCodeUnit() -> char
        {
            return *srcCursor;
        }

        // Consume a utf-8 char.
        auto ConsumeChar() -> void
        {
            ConsumeCodePointHelper();
            TryConsumeLineContinuation();
        }

        // Consume a utf-8 char and copy the code units to `buffer`.
        auto ConsumeChar(std::vector<char>& buffer) -> void
        {
            auto begin = srcCursor;
            ConsumeCodePointHelper();
            auto end = srcCursor;
            TryConsumeLineContinuation();

            buffer.insert(buffer.end(), begin, end);
        }

        // Consume a code unit requiring it's an ascii character. If not, return '\0' without consuming.
        auto ConsumeAsciiChar() -> char
        {
            const char firstChar = *srcCursor;
            if (firstChar == '\0' || (firstChar & 0x80)) [[unlikely]] {
                return '\0';
            }

            if (firstChar == '\n') {
                ++lineIndex;
                characterIndex = 0;
            }
            else {
                ++characterIndex;
            }

            ++srcCursor;
            TryConsumeLineContinuation();
            return firstChar;
        };

        // Try to consume an ascii char and return true if succeeded.
        // Assmumption: `ch` is an ascii character but not '\0' or '\n'
        auto TryConsumeAsciiChar(char ch) -> bool
        {
            GLSLD_ASSERT(ch != '\0' && ch != '\n');
            if (*srcCursor == ch) {
                ++srcCursor;
                ++characterIndex;
                TryConsumeLineContinuation();
                return true;
            }

            return false;
        }

        // Try to consume an ascii string and return true if succeeded.
        // Assumption: `text` is an ascii string without '\0' or '\n'
        // FIXME: line-continuation
        auto TryConsumeAsciiText(StringView text) -> bool
        {
            if (StringView{srcCursor, srcEnd}.StartWith(text)) {
                srcCursor += text.Size();
                characterIndex += text.Size();
                TryConsumeLineContinuation();
                return true;
            }

            return false;
        }

        auto CreateCheckPoint() -> ScannerCheckPoint
        {
            return ScannerCheckPoint{.srcCursor = srcCursor, .lineIndex = lineIndex, .characterIndex = characterIndex};
        }

        auto RestoreCheckPoint(const ScannerCheckPoint& checkpoint)
        {
            srcCursor      = checkpoint.srcCursor;
            lineIndex      = checkpoint.lineIndex;
            characterIndex = checkpoint.characterIndex;
        }

    private:
        auto ConsumeCodePointHelper() -> void
        {
            unsigned char firstChar = static_cast<unsigned char>(*srcCursor);
            if (firstChar == '\0') [[unlikely]] {
                return;
            }

            if (!(firstChar & 0x80)) [[likely]] {
                // Fast path for ascii characters
                ++srcCursor;

                if (firstChar == '\n') {
                    ++lineIndex;
                    characterIndex = 0;
                }
                else {
                    characterIndex += 1;
                }
            }
            else [[unlikely]] {
                int numCodeUnits = std::countl_one(firstChar);
                characterIndex += ComputeLspCodeUnitNum(numCodeUnits);
                srcCursor += numCodeUnits;

                if (srcCursor > srcEnd) [[unlikely]] {
                    srcCursor = srcEnd;
                }
            }
        }

        auto ComputeLspCodeUnitNum(int numCodeUnit) -> int
        {
            if (countUtf16Characters) {
                // utf-16
                return numCodeUnit == 4 ? 2 : 1;
            }
            else {
                // utf-8
                return numCodeUnit;
            }
        }

        auto TryConsumeLineContinuation() -> bool
        {
            bool consumed = false;
            while (srcCursor[0] == '\\') {
                if (srcCursor[1] == '\n') {
                    srcCursor += 2;
                    ++lineIndex;
                    characterIndex = 0;
                    consumed       = true;
                }
                else if (srcCursor[1] == '\r' && srcCursor[2] == '\n') {
                    srcCursor += 3;
                    ++lineIndex;
                    characterIndex = 0;
                    consumed       = true;
                }
                else {
                    break;
                }
            }

            return consumed;
        }

        // The beginning of the source string
        const char* srcBegin = nullptr;

        // The end of the source string, which is always a null terminator
        const char* srcEnd = nullptr;

        // The current cursor position in the source string
        const char* srcCursor = nullptr;

        int lineIndex      = 0;
        int characterIndex = 0;

        bool countUtf16Characters = false;
    };
} // namespace glsld