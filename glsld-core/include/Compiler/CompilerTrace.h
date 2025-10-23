#pragma once
#include "Basic/Print.h"
#include "Compiler/SyntaxToken.h"
#include "Support/Reflection.h"

#if defined(GLSLD_DEBUG)
#define GLSLD_ENABLE_COMPILER_TRACE 1
#endif

#if defined(GLSLD_ENABLE_COMPILER_TRACE)
namespace glsld
{
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

    class CompilerTrace
    {
    private:
    public:
        template <typename... Args>
        auto EmitTraceMessage(CompilerTraceSource source, fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            if (!CompilerTraceConfig::Get().traceCompiler) {
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

            DebugPrint(fmt, std::forward<Args>(args)...);
            DebugPrint("\n");
        }

        auto TracePPTokenLexed(const PPToken& tok) -> void
        {
            EmitTraceMessage(CompilerTraceSource::Lexer, "Lexed token [{}]'{}'", EnumToString(tok.klass),
                             tok.text.StrView());
        }

        auto TraceLexTokenIssued(const PPToken& tok, const TextRange& expandedRange) -> void
        {
            EmitTraceMessage(CompilerTraceSource::Preprocessor, "Issued token [{}]'{}' @ ({},{}~{},{})",
                             EnumToString(tok.klass), tok.text.StrView(), expandedRange.start.line,
                             expandedRange.start.character, expandedRange.end.line, expandedRange.end.character);
        }

        auto TraceEnterIncludeFile(StringView path) -> void
        {
            EmitTraceMessage(CompilerTraceSource::Preprocessor, "Entering include file '{}'", path);
        }

        auto TraceExitIncludeFile(StringView path) -> void
        {
            EmitTraceMessage(CompilerTraceSource::Preprocessor, "Exiting include file '{}'", path);
        }

        auto TraceTokenConsumed(const RawSyntaxToken& tok) -> void
        {
            EmitTraceMessage(CompilerTraceSource::Parser, "Consumed token [{}]'{}'", EnumToString(tok.klass),
                             tok.text.StrView());
        }

        // auto TraceAstCreated(const AstNode& node) -> void
        // {
        //     EmitTraceMessage(CompilerTraceSource::Parser, "Created Ast node [{}]'{}' @ ({},{}~{},{})",
        //                      TokenKlassToString(tok.klass), tok.text.StrView(), expandedRange.start.line,
        //                      expandedRange.start.character, expandedRange.end.line, expandedRange.end.character);
        // }
    };

    class ParserTrace
    {
    private:
        CompilerTrace& trace;

        // Name of the parser
        StringView name;

    public:
        ParserTrace(CompilerTrace& trace, StringView name) : trace(trace), name(name)
        {
            trace.EmitTraceMessage(CompilerTraceSource::Parser, "Entering parser {}", name);
        }
        ~ParserTrace()
        {
            trace.EmitTraceMessage(CompilerTraceSource::Parser, "Exiting parser {}", name);
        }
    };
} // namespace glsld
#endif
