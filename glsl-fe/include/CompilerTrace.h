#pragma once
#include "Common.h"
#include "SyntaxToken.h"
#include <fmt/format.h>

namespace glsld
{
#if defined(GLSLD_DEBUG_COMPILER)

    enum class CompilerTraceSource
    {
        Lexer,
        Preprocessor,
        Parser,
    };

    template <typename... Ts>
    inline auto EmitTraceMessage(CompilerTraceSource source, fmt::format_string<Ts...> fmt, Ts&&... args) -> void
    {
        StringView sourceName;
        switch (source) {
        case CompilerTraceSource::Lexer:
            fmt::print(stderr, "[Lexer] ");
            break;
        case CompilerTraceSource::Preprocessor:
            fmt::print(stderr, "[Preprocessor] ");
            break;
        case CompilerTraceSource::Parser:
            fmt::print(stderr, "[Parser] ");
            break;
        }

        fmt::print(stderr, fmt, std::forward<Ts>(args)...);
        fmt::print(stderr, "\n");
    }

    inline auto TracePPTokenLexed(const PPTokenData& tok) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Lexer, "Lexed token [{}]'{}'", TokenKlassToString(tok.klass),
                         tok.text.StrView());
    }

    inline auto TraceLexTokenIssued(const PPTokenData& tok) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Preprocessor, "Issued token [{}]'{}'", TokenKlassToString(tok.klass),
                         tok.text.StrView());
    }

    inline auto TraceEnterIncludeFile(StringView path) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Preprocessor, "Entering include file '{}'", path);
    }

    inline auto TraceExitIncludeFile(StringView path) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Preprocessor, "Exiting include file '{}'", path);
    }

    inline auto TraceTokenConsumed(const SyntaxToken& tok) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Parser, "Consumed token [{}]'{}'", TokenKlassToString(tok.klass),
                         tok.text.StrView());
    }

    inline auto TraceParserEnter(StringView name, std::initializer_list<TokenKlass> expect) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Parser, "Entering parser {}", name);
    }

    inline auto TraceParserExit(StringView name) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Parser, "Exiting parser {}", name);
    }

    class ParserTrace
    {
    public:
        ParserTrace(StringView name, std::initializer_list<TokenKlass> expect) : name(name)
        {
            TraceParserEnter(name, expect);
        }
        ~ParserTrace()
        {
            TraceParserExit(name);
        }

    private:
        StringView name;
    };

#define TRACE_TOKEN_CONSUMED(TOKEN) ::glsld::TraceTokenConsumed(TOKEN)
#define TRACE_TOKEN_SKIPPED(TOKEN) ::glsld::TraceTokenConsumed(TOKEN)
#define TRACE_PARSER(...)                                                                                              \
    ::glsld::ParserTrace glsld__parserTraceInstance                                                                    \
    {                                                                                                                  \
        __func__,                                                                                                      \
        {                                                                                                              \
            __VA_ARGS__                                                                                                \
        }                                                                                                              \
    }
#define TRACE_PARSE_CONTEXT()
#else
#define TRACE_TOKEN_CONSUMED(TOKEN)
#define TRACE_TOKEN_SKIPPED(TOKEN)
#define TRACE_PARSER(...)
#define TRACE_PARSE_CONTEXT()
#endif
} // namespace glsld