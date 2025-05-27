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
        // Name of this matcher. Used for composing error message.
        std::string name;

        std::optional<TokenKlass> expectedKlass;
        std::optional<std::string> expectedText;

    public:
        TokenMatcher(std::string name, std::optional<TokenKlass> klass, std::optional<std::string> text)
            : name(std::move(name)), expectedKlass(klass), expectedText(text)
        {
        }

        auto Describe() const -> std::string
        {
            return name;
        }

        auto Match(const AstSyntaxToken& token) const -> bool
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
} // namespace glsld