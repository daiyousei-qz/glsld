#include "Compiler.h"
#include "SourceContext.h"
#include "DiagnosticContext.h"
#include "LexContext.h"
#include "AstContext.h"
#include "TypeContext.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "AstPrinter.h"

namespace glsld
{
    CompiledPreamble::CompiledPreamble()  = default;
    CompiledPreamble::~CompiledPreamble() = default;

    static auto GetNextCompileId() -> int
    {
        static std::atomic_int nextId = 0;
        return nextId++;
    }

    CompilerObject::CompilerObject()  = default;
    CompilerObject::~CompilerObject() = default;

    auto CompilerObject::Initialize() -> void
    {
        compiled = false;
        moduleId = -1;

        sourceContext = nullptr;
        diagContext   = nullptr;
        lexContext    = nullptr;
        ppContext     = nullptr;
        astContext    = nullptr;
        typeContext   = nullptr;
        symbolTable   = nullptr;
    }

    auto CompilerObject::CreatePreamble() -> std::shared_ptr<CompiledPreamble>
    {
        // FIXME: actually it has to be a successful compilation
        GLSLD_REQUIRE(compiled);

        auto result         = std::make_shared<CompiledPreamble>();
        result->moduleId    = moduleId;
        result->lexContext  = std::move(lexContext);
        result->ppContext   = std::move(ppContext);
        result->astContext  = std::move(astContext);
        result->typeContext = std::move(typeContext);
        result->symbolTable = std::move(symbolTable);

        Initialize();
        return std::move(result);
    }

    auto CompilerObject::Compile(StringView sourceText) -> void
    {
        // Initialize context for compilation
        moduleId      = GetNextCompileId();
        sourceContext = std::make_unique<SourceContext>(GetFileSystemProvider());
        diagContext   = std::make_unique<DiagnosticContext>();
        lexContext    = std::make_unique<LexContext>();
        ppContext     = std::make_unique<PreprocessContext>();
        astContext    = std::make_unique<AstContext>();
        typeContext   = std::make_unique<TypeContext>();

        // Lexing
        {
            Preprocessor pp{*this, 0};
            Tokenizer{*this, pp, sourceText}.DoTokenize();
        }

        // Parsing
        {
            Parser{*this}.DoParse();
        }

        // Type checking
        {
            TypeChecker{*this}.TypeCheck(nullptr);
        }

#if defined(GLSLD_DEBUG)
        AstPrinter printer;
        printer.TraverseAst(*astContext);
        fmt::print(stderr, "{}", printer.Export());
#endif

        // Finalize compilation
        compiled = true;
    }
} // namespace glsld