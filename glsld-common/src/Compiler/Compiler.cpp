#include "Ast/AstDumper.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/SourceContext.h"
#include "Compiler/DiagnosticContext.h"
#include "Compiler/LexContext.h"
#include "Compiler/AstContext.h"
#include "Compiler/TypeContext.h"
#include "Compiler/Tokenizer.h"
#include "Compiler/Parser.h"

#include <atomic>

namespace glsld
{
    CompiledPreamble::CompiledPreamble()  = default;
    CompiledPreamble::~CompiledPreamble() = default;

    static auto GetNextCompileId() -> int
    {
        static std::atomic_int nextId = 0;
        return nextId++;
    }

    CompilerObject::CompilerObject()
    {
        Reset();
    }
    CompilerObject::~CompilerObject() = default;

    auto CompilerObject::Reset() -> void
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

        Reset();
        return std::move(result);
    }

    auto CompilerObject::CompileFromFile(StringView path, std::shared_ptr<CompiledPreamble> preamble,
                                         PPCallback* ppCallback) -> void
    {
        InitializeCompilation(std::move(preamble));
        sourceContext->SetMainFile(sourceContext->OpenFromFile(path.StdStrView()));

        DoCompile(ppCallback);

        FinalizeCompilation();
    }

    auto CompilerObject::CompileFromBuffer(StringView sourceText, std::shared_ptr<CompiledPreamble> preamble,
                                           PPCallback* ppCallback) -> void
    {
        InitializeCompilation(std::move(preamble));
        sourceContext->SetMainFile(sourceContext->OpenFromBuffer(sourceText));

        DoCompile(ppCallback);

        FinalizeCompilation();
    }

    auto CompilerObject::InitializeCompilation(std::shared_ptr<CompiledPreamble> preamble) -> void
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
    }

    auto CompilerObject::DoCompile(PPCallback* ppCallback) -> void
    {
        if (!sourceContext->GetMainFile()) {
            // FIXME: error
            GLSLD_UNREACHABLE();
        }

        auto mainFileEntry = sourceContext->GetMainFile();
        if (!mainFileEntry->GetSourceText().has_value()) {
            // FIXME: error
            GLSLD_UNREACHABLE();
        }

        // Lexing
        {
            GLSLD_TRACE_COMPILE_TIME("Lexing");
            Preprocessor pp{*this, ppCallback, std::nullopt};
            Tokenizer{*this, pp, mainFileEntry->GetID(), *mainFileEntry->GetSourceText()}.DoTokenize();
        }
        if (config.dumpTokens) {
            // FIXME:
        }

        // Parsing
        {
            GLSLD_TRACE_COMPILE_TIME("Parsing");
            Parser{*this}.DoParse();
        }
        if (config.dumpAst) {
            GetAstContext().GetTranslationUnit()->Print();
        }

        // Type checking
        {
            // GLSLD_TRACE_COMPILE_TIME("TypeChecking");
            // this->symbolTable =
            //     AstBuilder{*this}.DoTypeCheck(this->preamble ? this->preamble->symbolTable.get() : nullptr);
        }
    }

    auto CompilerObject::FinalizeCompilation() -> void
    {
        sourceContext->Finalize();

        compiled = true;
    }

} // namespace glsld