#pragma once

#include <memory>
#include <string>

namespace glsld
{
    class LexContext;
    class ParseContext;
    class DiagnosticContext;

    class GlsldCompiler
    {
    public:
        GlsldCompiler()
        {
        }

        auto Compile(std::string sourceText) -> void
        {
        }

    private:
        std::unique_ptr<DiagnosticContext> diagContext;
        std::unique_ptr<LexContext> lexContext;
        std::unique_ptr<ParseContext> parseContext;
    };
} // namespace glsld