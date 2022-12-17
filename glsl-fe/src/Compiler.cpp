#include "Compiler.h"

namespace glsld
{
    static auto GetNextCompileId() -> int
    {
        static std::atomic_int nextId = 0;
        return nextId++;
    }

    CompileResult::CompileResult(StringView sourceText, std::shared_ptr<CompiledDependency> module)
        : moduleId(GetNextCompileId())
    {
        diagContext = std::make_unique<DiagnosticContext>();
        lexContext  = std::make_unique<LexContext>();
        astContext  = std::make_unique<AstContext>();

        Tokenizer tokenizer{*lexContext, sourceText};
        tokenizer.DoTokenize();

        Parser parser{*lexContext, *astContext, *diagContext, moduleId};
        parser.DoParse();

        symbolTable = TypeChecker{}.TypeCheck(*astContext, module ? &module->GetSymbolTable() : nullptr);

        if (module) {
            dependentModules.push_back(std::move(module));
        }

#if defined(GLSLD_DEBUG)
        AstPrinter printer;
        printer.TraverseAst(*astContext);
        fmt::print(stderr, "{}", printer.Export());
#endif
    }

    CompileResult::CompileResult(CompileResult&& other)
    {
        *this = std::move(other);
    }
    auto CompileResult::operator=(CompileResult&& other) -> CompileResult&
    {
        moduleId         = std::move(other.moduleId);
        diagContext      = std::move(other.diagContext);
        lexContext       = std::move(other.lexContext);
        astContext       = std::move(other.astContext);
        symbolTable      = std::move(other.symbolTable);
        dependentModules = std::move(other.dependentModules);

        other.moduleId = -1;
        return *this;
    }

    auto Compile(StringView sourceText, std::shared_ptr<CompiledDependency> module) -> std::unique_ptr<CompileResult>
    {
        return std::unique_ptr<CompileResult>{new CompileResult{sourceText, std::move(module)}};
    }

} // namespace glsld