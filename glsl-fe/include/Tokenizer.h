#pragma once
#include "Common.h"
#include "LexContext.h"
#include "SyntaxToken.h"

#include <unordered_map>
#include <vector>

namespace glsld
{
    struct TokenInfo
    {
        TokenKlass klass;
        TextPosition begin;
        TextPosition end;
    };

    class Tokenizer
    {
    public:
        Tokenizer(LexContext& lexContext, StringView source)
            : lexContext(lexContext), srcBegin(source.Data()), srcEnd(source.Data() + source.Size()),
              lineCursor(source.Data())
        {
            // Load first line and calibrate line index to be zero-based
            LoadLine();
            lineIndex = 0;
        }

        auto DoTokenize() -> void
        {
            while (NextToken()) {
            }
        }

    private:
        auto Eof() -> bool
        {
            return lineCursor == srcEnd && lineBuffer.empty();
        }

        auto NextToken() -> bool;

        auto GetCurrentTextPosition() -> TextPosition;

        auto ParseLineComment() -> TokenKlass;

        auto ParseBlockComment() -> TokenKlass;

        auto ParseRegularToken() -> TokenKlass;

        auto PeekChar() -> char;

        auto PeekChar(size_t lookahead) -> char;

        auto SkipWhitespace() -> void;

        auto LoadLine() -> void;

        LexContext& lexContext;

        const char* srcBegin;
        const char* srcEnd;
        const char* lineCursor;

        bool freshLine     = true;
        size_t lineIndex   = 0;
        size_t bufferIndex = 0;
        std::vector<char> lineBuffer;
        std::vector<size_t> lineBreakBuffer;

        std::string tokenBuffer;
    };
} // namespace glsld
