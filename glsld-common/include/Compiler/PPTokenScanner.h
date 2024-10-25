#pragma once
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    // A simple scanner for a preprosessing token stream. It is used to parse preprocessor directives.
    class PPTokenScanner final
    {
    private:
        const PPToken* tokBegin  = nullptr;
        const PPToken* tokEnd    = nullptr;
        const PPToken* tokCursor = nullptr;

    public:
        PPTokenScanner() = default;
        PPTokenScanner(ArrayView<const PPToken> tokens)
            : tokBegin(tokens.data()), tokEnd(tokens.data() + tokens.size()), tokCursor(tokBegin)
        {
        }
        PPTokenScanner(const PPToken* tokBegin, const PPToken* tokEnd)
            : tokBegin(tokBegin), tokEnd(tokEnd), tokCursor(tokBegin)
        {
        }

        auto CursorAtBegin() const noexcept -> bool
        {
            return tokCursor == tokBegin;
        }

        auto CursorAtEnd() const noexcept -> bool
        {
            return tokCursor == tokEnd;
        }

        auto RemainingTokenCount() const noexcept -> size_t
        {
            return tokEnd - tokCursor;
        }

        auto PeekToken() const noexcept -> const PPToken&
        {
            return *tokCursor;
        }

        auto TryTestToken(TokenKlass klass) -> bool
        {
            if (!CursorAtEnd() && tokCursor->klass == klass) {
                return true;
            }

            return false;
        }

        auto TryTestToken(TokenKlass klass, size_t lookahead) -> bool
        {
            if (RemainingTokenCount() > lookahead && tokCursor[lookahead].klass == klass) {
                return true;
            }

            return false;
        }

        auto TryConsumeToken(TokenKlass klass) -> std::optional<PPToken>
        {
            if (!CursorAtEnd() && tokCursor->klass == klass) {
                return ConsumeToken();
            }

            return std::nullopt;
        }

        auto TryConsumeToken(TokenKlass klass1, TokenKlass klass2) -> std::optional<PPToken>
        {
            if (!CursorAtEnd() && (tokCursor->klass == klass1 || tokCursor->klass == klass2)) {
                return ConsumeToken();
            }

            return std::nullopt;
        }

        auto ConsumeToken() -> PPToken
        {
            return *tokCursor++;
        }
    };
} // namespace glsld