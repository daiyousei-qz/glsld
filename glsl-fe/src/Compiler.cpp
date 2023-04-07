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

    auto CompilerObject::Compile(StringView sourceText, std::shared_ptr<CompiledPreamble> preamble) -> void
    {
        // Initialize context for compilation
        this->moduleId      = GetNextCompileId();
        this->preamble      = std::move(preamble);
        this->sourceContext = std::make_unique<SourceContext>(GetFileSystemProvider());
        this->diagContext   = std::make_unique<DiagnosticContext>();
        this->lexContext    = std::make_unique<LexContext>();
        this->ppContext     = std::make_unique<PreprocessContext>();
        this->astContext    = std::make_unique<AstContext>();
        this->typeContext   = std::make_unique<TypeContext>();

        // Lexing
        {
            GLSLD_TRACE_COMPILE_TIME("Lexing");
            Preprocessor pp{*this, 0};
            Tokenizer{*this, pp, sourceText}.DoTokenize();
        }

        // Parsing
        {
            GLSLD_TRACE_COMPILE_TIME("Parsing");
            Parser{*this}.DoParse();
        }

        // Type checking
        {
            GLSLD_TRACE_COMPILE_TIME("TypeChecking");
            this->symbolTable =
                TypeChecker{*this}.TypeCheck(this->preamble ? this->preamble->symbolTable.get() : nullptr);
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