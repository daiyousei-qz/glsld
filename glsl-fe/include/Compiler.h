#pragma once

#include "DiagnosticContext.h"
#include "LexContext.h"
#include "AstContext.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "AstPrinter.h"
#include "SourceInfo.h"

#include <memory>
#include <string>

namespace glsld
{
    class CompiledDependency;

    class CompileResult
    {
    public:
        CompileResult(StringView sourceText, std::shared_ptr<CompiledDependency> module = nullptr);

        CompileResult(const CompileResult&)                    = delete;
        auto operator=(const CompileResult&) -> CompileResult& = delete;

        CompileResult(CompileResult&&);
        auto operator=(CompileResult&&) -> CompileResult&;

        auto GetId() const -> int
        {
            return moduleId;
        }

        auto IsValid() const -> bool
        {
            return moduleId >= 0;
        }

        auto HasDependency() const -> bool
        {
            return !dependentModules.empty();
        }

        auto GetDiagnosticContext() const -> const DiagnosticContext&
        {
            return *diagContext;
        }
        auto GetLexContext() const -> const LexContext&
        {
            return *lexContext;
        }
        auto GetAstContext() const -> const AstContext&
        {
            return *astContext;
        }
        auto GetSymbolTable() const -> const SymbolTable&
        {
            return symbolTable;
        }

    private:
        friend auto Compile(StringView, std::shared_ptr<CompiledDependency>) -> std::unique_ptr<CompileResult>;

        int moduleId = -1;

        std::unique_ptr<DiagnosticContext> diagContext;
        std::unique_ptr<LexContext> lexContext;
        std::unique_ptr<AstContext> astContext;

        SymbolTable symbolTable;

        std::vector<std::shared_ptr<CompiledDependency>> dependentModules;
    };

    class CompiledDependency : public std::enable_shared_from_this<CompiledDependency>
    {
    public:
        CompiledDependency(std::unique_ptr<CompileResult> data) : compileResult(std::move(*data))
        {
            GLSLD_ASSERT(!this->compileResult.HasDependency());
        }

        auto GetId() const -> int
        {
            return compileResult.GetId();
        }

        auto GetDiagnosticContext() const -> const DiagnosticContext&
        {
            return compileResult.GetDiagnosticContext();
        }
        auto GetLexContext() const -> const LexContext&
        {
            return compileResult.GetLexContext();
        }
        auto GetAstContext() const -> const AstContext&
        {
            return compileResult.GetAstContext();
        }
        auto GetSymbolTable() const -> const SymbolTable&
        {
            return compileResult.GetSymbolTable();
        }

    private:
        CompileResult compileResult;
    };

    auto Compile(StringView sourceText, std::shared_ptr<CompiledDependency> module = nullptr)
        -> std::unique_ptr<CompileResult>;

    template <typename Derived>
    class ModuleVisitor : public AstVisitor<Derived>
    {
    public:
        ModuleVisitor(const CompileResult& data) : data(&data)
        {
        }

        auto Traverse() -> void
        {
            AstVisitor<Derived>::TraverseAst(data->GetAstContext());
        }

        using AstVisitor<Derived>::Traverse;

    protected:
        auto GetLexContext() const -> const LexContext&
        {
            return data->GetLexContext();
        }
        auto GetAstContext() const -> const AstContext&
        {
            return data->GetAstContext();
        }

        auto EnterIfContainsPosition(AstNodeBase& node, TextPosition position) -> AstVisitPolicy
        {
        }
        auto EnterIfOverlapRange(AstNodeBase& node, TextRange range) -> AstVisitPolicy
        {
        }

    private:
        const CompileResult* data;
    };

} // namespace glsld