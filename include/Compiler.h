#pragma once

#include "DiagnosticContext.h"
#include "LexContext.h"
#include "ParseContext.h"

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
            GLSLD_ASSERT(!compiled);

            diagContext  = std::make_unique<DiagnosticContext>();
            lexContext   = std::make_unique<LexContext>(std::move(sourceText));
            parseContext = std::make_unique<ParseContext>(diagContext.get(), lexContext.get());

            parseContext->DoParseTranslationUnit();
            compiled = true;
        }

        auto GetAst() -> const ParsedAst*
        {
            GLSLD_ASSERT(compiled);
            return parseContext->GetAst();
        }

    private:
        bool compiled = false;

        std::unique_ptr<DiagnosticContext> diagContext = nullptr;
        std::unique_ptr<LexContext> lexContext         = nullptr;
        std::unique_ptr<ParseContext> parseContext     = nullptr;
    };
} // namespace glsld