#pragma once
#include "Basic/Common.h"
#include "Basic/Print.h"
#include "Compiler/SyntaxToken.h"

#include <chrono>

namespace glsld
{
#if defined(GLSLD_DEBUG)
#define GLSLD_ENABLE_COMPILER_TRACE 1
#endif

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
    struct CompilerTraceConfig
    {
        bool traceCompiler = false;

        static auto Get() -> CompilerTraceConfig&
        {
            static CompilerTraceConfig config = []() {
                CompilerTraceConfig result;
                if (auto envTraceCompiler = std::getenv("__GLSLD_TRACE_COMPILER")) {
                    result.traceCompiler = std::stoi(envTraceCompiler) != 0;
                }

                return result;
            }();
            return config;
        }
    };

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
        if (CompilerTraceConfig::Get().traceCompiler == false) {
            return;
        }

        StringView sourceName;
        switch (source) {
        case CompilerTraceSource::Lexer:
            DebugPrint("[Lexer] ");
            break;
        case CompilerTraceSource::Preprocessor:
            DebugPrint("[Preprocessor] ");
            break;
        case CompilerTraceSource::Parser:
            DebugPrint("[Parser] ");
            break;
        case CompilerTraceSource::Compiler:
            DebugPrint("[Compiler] ");
            break;
        }

        DebugPrint(fmt, std::forward<Ts>(args)...);
        DebugPrint("\n");
    }

    inline auto TracePPTokenLexed(const PPToken& tok) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Lexer, "Lexed token [{}]'{}'", TokenKlassToString(tok.klass),
                         tok.text.StrView());
    }

    inline auto TraceLexTokenIssued(const PPToken& tok, const TextRange& expandedRange) -> void
    {
        EmitTraceMessage(CompilerTraceSource::Preprocessor, "Issued token [{}]'{}' @ ({},{}~{},{})",
                         TokenKlassToString(tok.klass), tok.text.StrView(), expandedRange.start.line,
                         expandedRange.start.character, expandedRange.end.line, expandedRange.end.character);
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

    // inline auto TraceAstCreated(const AstNode& node) -> void
    // {
    //     EmitTraceMessage(CompilerTraceSource::Parser, "Created Ast node [{}]'{}' @ ({},{}~{},{})",
    //                      TokenKlassToString(tok.klass), tok.text.StrView(), expandedRange.start.line,
    //                      expandedRange.start.character, expandedRange.end.line, expandedRange.end.character);
    // }

    class ParserTrace
    {
    public:
        ParserTrace(StringView name) : name(name)
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
#define GLSLD_TRACE_TOKEN_ISSUED(TOKEN, EXPANDED_RANGE) ::glsld::TraceLexTokenIssued(TOKEN, EXPANDED_RANGE)
#define GLSLD_TRACE_ENTER_INCLUDE_FILE(PATH) ::glsld::TraceEnterIncludeFile(PATH)
#define GLSLD_TRACE_EXIT_INCLUDE_FILE(PATH) ::glsld::TraceExitIncludeFile(PATH)
#define GLSLD_TRACE_TOKEN_CONSUMED(TOKEN) ::glsld::TraceTokenConsumed(TOKEN)
#define GLSLD_TRACE_TOKEN_SKIPPED(TOKEN) ::glsld::TraceTokenConsumed(TOKEN)
#define GLSLD_TRACE_PARSER()                                                                                           \
    ::glsld::ParserTrace GLSLD_TRACE_OBJECT_NAME(parserTraceObject)                                                    \
    {                                                                                                                  \
        __func__                                                                                                       \
    }
#define GLSLD_TRACE_COMPILE_TIME(NAME)                                                                                 \
    ::glsld::CompileTimeTrace GLSLD_TRACE_OBJECT_NAME(compileTimeTraceObject)                                          \
    {                                                                                                                  \
        NAME                                                                                                           \
    }
#else
#define GLSLD_TRACE_TOKEN_LEXED(TOKEN)
#define GLSLD_TRACE_TOKEN_ISSUED(TOKEN, EXPANDED_RANGE)
#define GLSLD_TRACE_ENTER_INCLUDE_FILE(PATH)
#define GLSLD_TRACE_EXIT_INCLUDE_FILE(PATH)
#define GLSLD_TRACE_TOKEN_CONSUMED(TOKEN)
#define GLSLD_TRACE_TOKEN_SKIPPED(TOKEN)
#define GLSLD_TRACE_PARSER(...)
#define GLSLD_TRACE_COMPILE_TIME(NAME)
#endif
} // namespace glsld