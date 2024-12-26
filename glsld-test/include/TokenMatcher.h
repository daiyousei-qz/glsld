#pragma once
#include "Basic/StringView.h"
#include "Compiler/SyntaxToken.h"

#include <optional>
#include <string>

namespace glsld
{
    class TokenMatcher
    {
    private:
        // Description of this matcher. Used for composing error message.
        std::string desc;

        std::optional<TokenKlass> expectedKlass;
        std::optional<std::string> expectedText;

    public:
        TokenMatcher(std::string desc, std::optional<TokenKlass> klass, std::optional<std::string> text)
            : desc(std::move(desc)), expectedKlass(klass), expectedText(text)
        {
        }

        auto Describe() const -> std::string
        {
            return desc;
        }

        auto Match(const SyntaxToken& token) const -> bool
        {
            if (expectedKlass && token.klass != *expectedKlass) {
                return false;
            }
            if (expectedText && token.text != *expectedText) {
                return false;
            }

            return true;
        }
    };

    inline auto AnyTok() -> TokenMatcher
    {
        return TokenMatcher("AnyToken", std::nullopt, std::nullopt);
    }
    inline auto EofTok() -> TokenMatcher
    {
        return TokenMatcher("EofToken", TokenKlass::Eof, std::nullopt);
    }
    inline auto InvalidTok() -> TokenMatcher
    {
        return TokenMatcher("InvalidToken", TokenKlass::Invalid, std::nullopt);
    }
    inline auto IdTok(StringView identifier) -> TokenMatcher
    {
        return TokenMatcher("Identifier", TokenKlass::Identifier, identifier.Str());
    }
    inline auto KeywordTok(TokenKlass keyword) -> TokenMatcher
    {
        return TokenMatcher("Keyword", keyword, std::nullopt);
    }
    inline auto IntTok(StringView value) -> TokenMatcher
    {
        return TokenMatcher("IntegerConstant", TokenKlass::IntegerConstant, value.Str());
    }
    inline auto FloatTok(StringView value) -> TokenMatcher
    {
        return TokenMatcher("FloatConstant", TokenKlass::FloatConstant, value.Str());
    }
} // namespace glsld