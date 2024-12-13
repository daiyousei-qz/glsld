#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Basic/SourceInfo.h"

#include <vector>

namespace glsld
{
    struct ScannerCheckPoint
    {
        const char* srcCursor;
        int lineIndex;
        int characterIndex;
    };

    // A text scanner that provides utilities for lexing the source text.
    // It is required that the source string provided is utf-8 encoded and null-terminated.
    class SourceScanner
    {
    private:
        // The beginning of the source string
        const char* srcBegin = nullptr;

        // The end of the source string, which is always a null terminator (valid to access).
        const char* srcEnd = nullptr;

        // The current cursor position in the source string
        const char* srcCursor = nullptr;

        // The line number of the current cursor position.
        int lineCounter = 0;

        // The character offset in the current line of the current cursor position.
        int characterCounter = 0;

        // Count in utf-16 code units instead of utf-8. LSP requires utf-16 code units.
        bool countUtf16Characters = false;

    public:
        SourceScanner() = default;
        SourceScanner(const char* srcBegin, const char* srcEnd, bool countUtf16Characters)
            : srcBegin(srcBegin), srcEnd(srcEnd), srcCursor(srcBegin), countUtf16Characters(countUtf16Characters)
        {
            // FIXME:
            // TryConsumeLineContinuation();
        }
        SourceScanner(StringView src, bool countUtf16Characters)
            : SourceScanner(src.data(), src.data() + src.Size(), countUtf16Characters)
        {
        }

        // True if the cursor is at the beginning of the source.
        auto CursorAtBegin() const noexcept -> bool
        {
            return srcCursor == srcBegin;
        }

        // True if the cursor is at the end of the source.
        auto CursorAtEnd() const noexcept -> bool
        {
            return srcCursor == srcEnd;
        }

        auto GetScannedText() const noexcept -> StringView
        {
            return StringView{srcBegin, srcCursor};
        }

        auto GetRemainingText() const noexcept -> StringView
        {
            return StringView{srcCursor, srcEnd};
        }

        // Get the current cursor position in the source text in line number and character offset in the line.
        auto GetTextPosition() const noexcept -> TextPosition
        {
            return TextPosition{lineCounter, characterCounter};
        };

        // Consumes all whitespace characters from the current cursor position.
        auto SkipWhitespace(bool& skippedWhitespace, bool& skippedNewline) -> void
        {
            while (!CursorAtEnd()) {
                auto ch = *srcCursor;

                if (ch == ' ' || ch == '\t' || ch == '\r') {
                    ++characterCounter;

                    skippedWhitespace = true;
                    ++srcCursor;
                }
                else if (ch == '\n') {
                    ++lineCounter;
                    characterCounter = 0;

                    skippedWhitespace = true;
                    skippedNewline    = true;
                    ++srcCursor;
                }
                else if (!TryConsumeLineContinuation()) {
                    break;
                }
            }
        }

        // Consumes all characters until the next '#' character at the beginning of a line.
        auto SkipUntilPPHash() -> void
        {
            bool startOfLine = false;
            while (!CursorAtEnd()) {
                auto ch = *srcCursor;

                if (ch == '\n') {
                    if (!TryConsumeLineContinuation()) {
                        startOfLine = true;
                    }
                    ++lineCounter;
                    characterCounter = 0;
                }
                else if (ch == ' ' || ch == '\t' || ch == '\r') {
                    ++characterCounter;
                }
                else if (ch == '#') {
                    if (startOfLine) {
                        break;
                    }
                }
                else {
                    startOfLine = false;
                }

                ConsumeChar();
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
                ++lineCounter;
                characterCounter = 0;
            }
            else {
                ++characterCounter;
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
                ++characterCounter;
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
                characterCounter += text.Size();
                TryConsumeLineContinuation();
                return true;
            }

            return false;
        }

        // Creates a checkpoint that can be used to restore the scanner state later. Necessary for backtracking.
        auto CreateCheckPoint() -> ScannerCheckPoint
        {
            return ScannerCheckPoint{
                .srcCursor = srcCursor, .lineIndex = lineCounter, .characterIndex = characterCounter};
        }

        // Restores the scanner state to the checkpoint.
        auto RestoreCheckPoint(const ScannerCheckPoint& checkpoint)
        {
            srcCursor        = checkpoint.srcCursor;
            lineCounter      = checkpoint.lineIndex;
            characterCounter = checkpoint.characterIndex;
        }

    private:
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

        // Consume a utf-8 code point from the source string.
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
                    ++lineCounter;
                    characterCounter = 0;
                }
                else {
                    characterCounter += 1;
                }
            }
            else [[unlikely]] {
                // Slow path for non-ascii characters. We don't validate the encoding for better performance.
                int numCodeUnits = std::countl_one(firstChar);
                characterCounter += ComputeLspCodeUnitNum(numCodeUnits);
                srcCursor += numCodeUnits;

                if (srcCursor > srcEnd) [[unlikely]] {
                    srcCursor = srcEnd;
                }
            }
        }

        auto TryConsumeLineContinuation() -> bool
        {
            bool consumed = false;
            while (srcCursor[0] == '\\') {
                if (srcCursor[1] == '\n') {
                    srcCursor += 2;
                    ++lineCounter;
                    characterCounter = 0;
                    consumed         = true;
                }
                else if (srcCursor[1] == '\r' && srcCursor[2] == '\n') {
                    srcCursor += 3;
                    ++lineCounter;
                    characterCounter = 0;
                    consumed         = true;
                }
                else {
                    break;
                }
            }

            return consumed;
        }
    };
} // namespace glsld