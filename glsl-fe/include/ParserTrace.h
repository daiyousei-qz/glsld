#pragma once
#include "SyntaxToken.h"
#include <fmt/format.h>

namespace glsld
{
// #define GLSLD_DEBUG_PARSER 1
#if defined(GLSLD_DEBUG_PARSER)

    inline auto TraceTokenConsumed(const SyntaxToken& tok) -> void
    {
        fmt::print(stderr, "Consumed token [{}]'{}'\n", TokenKlassToString(tok.klass), tok.text.Get());
    }

    inline auto TraceParserEnter(std::string_view name, std::initializer_list<TokenKlass> expect) -> void
    {
        fmt::print(stderr, "Entering parser {}\n", name);
    }

    inline auto TraceParserExit(std::string_view name) -> void
    {
        fmt::print(stderr, "Exiting parser {}\n", name);
    }

    class ParserTrace
    {
    public:
        ParserTrace(std::string_view name, std::initializer_list<TokenKlass> expect) : name(name)
        {
            TraceParserEnter(name, expect);
        }
        ~ParserTrace()
        {
            TraceParserExit(name);
        }

    private:
        std::string_view name;
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