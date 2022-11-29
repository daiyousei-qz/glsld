#pragma once

#include "DiagnosticContext.h"
#include "LexContext.h"
#include "AstContext.h"
#include "Parser.h"
#include "AstPrinter.h"
#include "TypeChecker.h"

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

            diagContext = std::make_unique<DiagnosticContext>();
            lexContext  = std::make_unique<LexContext>(std::move(sourceText));
            astContext  = std::make_unique<AstContext>();

            Parser parser{lexContext.get(), astContext.get(), diagContext.get()};
            parser.DoParseTranslationUnit();
            compiled = true;

            TypeChecker{}.TraverseAst(*astContext);
            AstPrinter{}.TraverseAst(*astContext);
        }

        auto GetDiagnosticContext() -> const DiagnosticContext&
        {
            return *diagContext;
        }
        auto GetLexContext() -> const LexContext&
        {
            return *lexContext;
        }
        auto GetAstContext() -> const AstContext&
        {
            return *astContext;
        }

    private:
        bool compiled = false;

        std::unique_ptr<DiagnosticContext> diagContext = nullptr;
        std::unique_ptr<LexContext> lexContext         = nullptr;
        std::unique_ptr<AstContext> astContext         = nullptr;
    };
} // namespace glsld