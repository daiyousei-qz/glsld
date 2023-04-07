#pragma once
#include "Common.h"
#include "SyntaxToken.h"

#include <chrono>
#include <fmt/format.h>

namespace glsld
{
#define GLSLD_DEBUG_COMPILER 1

#if defined(GLSLD_DEBUG_COMPILER)

    enum class CompilerTraceSource
    {
        Lexer,
        Preprocessor,
        Parser,
        Compiler,
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
        case CompilerTraceSource::Compiler:
            fmt::print(stderr, "[Compiler] ");
            break;
        }

        fmt::print(stderr, fmt, std::forward<Ts>(args)...);
        fmt::print(stderr, "\n");
    }

    inline auto TracePPTokenLexed(const PPToken& tok) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Lexer, "Lexed token [{}]'{}'", TokenKlassToString(tok.klass),
                         tok.text.StrView());
    }

    inline auto TraceLexTokenIssued(const PPToken& tok) -> void
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

    class ParserTrace
    {
    public:
        ParserTrace(StringView name, std::initializer_list<TokenKlass> expect) : name(name)
        {
            EmitTraceMessage(CompilerTraceSource::Parser, "Entering parser {}", name);
        }
        ~ParserTrace()
        {
            EmitTraceMessage(CompilerTraceSource::Parser, "Exiting parser {}", name);
        }

    private:
        StringView name;
    };

    class CompileTimeTrace
    {
    public:
        CompileTimeTrace(StringView name) : name(name), t0(std::chrono::steady_clock::now())
        {
        }
        ~CompileTimeTrace()
        {
            auto t1       = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
            EmitTraceMessage(CompilerTraceSource::Compiler, "Compile time of {}: {}ms", name, duration.count());
        }

    private:
        using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
        StringView name;
        TimePoint t0;
    };

#define GLSLD_TRACE_OBJECT_NAME_AUX(NAME, LINE) glsld__##NAME##LINE
#define GLSLD_TRACE_OBJECT_NAME(NAME) GLSLD_TRACE_OBJECT_NAME_AUX(NAME, __LINE__)

#define GLSLD_TRACE_TOKEN_LEXED(TOKEN) ::glsld::TracePPTokenLexed(TOKEN)
#define GLSLD_TRACE_TOKEN_ISSUED(TOKEN) ::glsld::TraceLexTokenIssued(TOKEN)
#define GLSLD_TRACE_ENTER_INCLUDE_FILE(PATH) ::glsld::TraceEnterIncludeFile(PATH)
#define GLSLD_TRACE_EXIT_INCLUDE_FILE(PATH) ::glsld::TraceExitIncludeFile(PATH)
#define GLSLD_TRACE_TOKEN_CONSUMED(TOKEN) ::glsld::TraceTokenConsumed(TOKEN)
#define GLSLD_TRACE_TOKEN_SKIPPED(TOKEN) ::glsld::TraceTokenConsumed(TOKEN)
#define GLSLD_TRACE_PARSER(...)                                                                                        \
    ::glsld::ParserTrace GLSLD_TRACE_OBJECT_NAME(parserTraceObject)                                                    \
    {                                                                                                                  \
        __func__,                                                                                                      \
        {                                                                                                              \
            __VA_ARGS__                                                                                                \
        }                                                                                                              \
    }
#define GLSLD_TRACE_COMPILE_TIME(NAME)                                                                                 \
    ::glsld::CompileTimeTrace GLSLD_TRACE_OBJECT_NAME(compileTimeTraceObject)                                          \
    {                                                                                                                  \
        NAME                                                                                                           \
    }
#else
#define GLSLD_TRACE_TOKEN_LEXED(TOKEN)
#define GLSLD_TRACE_TOKEN_ISSUED(TOKEN)
#define GLSLD_TRACE_ENTER_INCLUDE_FILE(PATH)
#define GLSLD_TRACE_EXIT_INCLUDE_FILE(PATH)
#define GLSLD_TRACE_TOKEN_CONSUMED(TOKEN)
#define GLSLD_TRACE_TOKEN_SKIPPED(TOKEN)
#define GLSLD_TRACE_PARSER(...)
#define GLSLD_TRACE_COMPILE_TIME(NAME)
#endif
} // namespace glsld