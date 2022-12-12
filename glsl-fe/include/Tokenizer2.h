#pragma once
#include "Common.h"
#include "SyntaxToken.h"

namespace glsld
{
    struct TokenInfo
    {
        TokenKlass klass;

        // NOTE these are all 0-based
        int lineBegin;
        int lineEnd;
        int columnBegin;
        int columnEnd;
    };

    class Tokenizer2
    {
    public:
        auto NextToken(std::string& buf) -> TokenInfo
        {
            return TokenInfo{
                .klass = TokenKlass::Eof,
            };
        }

    private:
        auto LoadLine() -> void
        {
            auto line = remainingText.TakeUntil([](char ch) { return ch == '\n'; });
        }

        StringView currentLineText;
        StringView remainingText;
    };
} // namespace glsld