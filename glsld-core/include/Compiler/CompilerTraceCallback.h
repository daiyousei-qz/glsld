#pragma once
#include "Basic/AtomTable.h"
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    class CompilerTraceCallback
    {
    public:
        CompilerTraceCallback()          = default;
        virtual ~CompilerTraceCallback() = default;

        // Called when a token is lexed from the source text.
        virtual auto TokenLexed(TokenKlass klass, const TextRange& spelledRange, AtomString text) -> void
        {
        }

        // Called when a token is issued by the preprocessor.
        virtual auto TokenIssued(TokenKlass klass, const TextRange& expandedRange, AtomString text) -> void
        {
        }

        // Called when a token is consumed by the parser.
        virtual auto TokenConsumed(TokenKlass klass, AtomString text) -> void
        {
        }
    };
} // namespace glsld