#pragma once
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Support/StringView.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/Preprocessor.h"

namespace glsld
{
    class Tokenizer final
    {
    private:
        const PreprocessStateMachine& pp;

        FileID sourceFile;

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

        std::vector<char> tokenTextBuffer;

        // Peek the next code unit.
        auto PeekCodeUnit() -> char
        {
            return *srcCursor;
        }

        // Get the current cursor position in the source text in line number and character offset in the line.
        auto GetTextPosition() const noexcept -> TextPosition
        {
            return TextPosition{lineCounter, characterCounter};
        };

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

        auto ExtractTokenText() -> AtomString
        {
            return pp.GetAtomTable().GetAtom(StringView(tokenTextBuffer.data(), tokenTextBuffer.size()));
        }

        auto TryConsumeLineContinuation() -> bool;

        // Consumes all whitespace characters from the current cursor position.
        auto TryConsumeWhitespace(bool& skippedWhitespace, bool& skippedNewline) -> void;

        // Consume a utf-8 char and returns a view of the consumed code units. It is guaranteed that this function
        // returns a non-empty view. If EOF is reached, a view to the null terminator will be returned.
        // Note that we don't validate the utf-8 encoding.
        auto ConsumeChar(TextPosition& endPos) -> StringView;

        // Consume a utf-8 char and returns a view of the consumed code units, assuming it is not '\n' nor '\0'.
        // Note that we don't validate the utf-8 encoding.
        auto ConsumeCharUnsafe(TextPosition& endPos) -> StringView;

        // Consume an ascii char and returns it, assuming it is ASCII but not '\n' nor '\0'.
        auto ConsumeAsciiCharUnsafe(TextPosition& endPos) -> char;

        // Try to consume the current char if it equals the given char, but it cannot be '\n' nor '\0'.
        auto TryConsumeAsciiChar(TextPosition& endPos, char ch) -> bool;

        // Assuming we are seeing an underscore or alphabetic character, parse the identifier.
        auto LexIdentifier(TextPosition& endPos, char firstChar) -> std::tuple<TokenKlass, AtomString>;

        // Assuming we are seeing a dot or digit character, parse the number literal.
        auto LexNumberLiteral(TextPosition& endPos, char firstChar) -> std::tuple<TokenKlass, AtomString>;

        // Assuming we are seeing "//", parse the line comment and return the token klass.
        auto LexLineComment(TextPosition& endPos) -> std::tuple<TokenKlass, AtomString>;

        // Assuming we are seeing "/*", parse the block comment and return the token klass.
        auto LexBlockComment(TextPosition& endPos) -> std::tuple<TokenKlass, AtomString>;

        // Assuming we are seeing `quoteStart`, parse the header name closed by `quoteEnd` and return the token klass.
        auto LexHeaderName(TextPosition& endPos, char quoteStart, char quoteEnd, TokenKlass klass)
            -> std::tuple<TokenKlass, AtomString>;

    public:
        Tokenizer(const PreprocessStateMachine& pp, FileID sourceFile, SourceTextView sourceText,
                  bool countUtf16Characters)
            : pp(pp), sourceFile(sourceFile), srcBegin(sourceText.begin()), srcEnd(sourceText.end()),
              srcCursor(sourceText.begin()), countUtf16Characters(countUtf16Characters)
        {
            GLSLD_ASSERT(*srcEnd == '\0');
        }

        auto Exhausted() const -> bool
        {
            return srcCursor == srcEnd;
        }

        // Skip leading whitespace and lex the next token.
        // If the end of the source is reached, an EOF token will be returned.
        auto Lex() -> PPToken;
    };
} // namespace glsld