#pragma once

#include "DiagnosticContext.h"
#include "LexContext.h"
#include "AstContext.h"
#include "Parser.h"
#include "TypeChecker.h"
#include "AstPrinter.h"

#include <memory>
#include <string>

namespace glsld
{
    class LexContext;
    class ParseContext;
    class DiagnosticContext;

    class CompiledModule;
    class CompiledExternalModule;

    namespace detail
    {
        class CompiledModuleImpl
        {
        public:
            CompiledModuleImpl() = default;

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

        protected:
            std::unique_ptr<DiagnosticContext> diagContext = nullptr;
            std::unique_ptr<LexContext> lexContext         = nullptr;
            std::unique_ptr<AstContext> astContext         = nullptr;

            // For global symbols
            SymbolTable symbolTable;
        };
    } // namespace detail

    class CompiledModule : public detail::CompiledModuleImpl
    {
    public:
        using CompiledModuleImpl::CompiledModuleImpl;

        auto GetDependentModule() -> const CompiledExternalModule*
        {
            return dependentModule.get();
        }

    private:
        friend class GlslCompiler;
        friend class CompiledExternalModule;

        std::shared_ptr<CompiledExternalModule> dependentModule;
    };

    class CompiledExternalModule : public detail::CompiledModuleImpl
    {
    public:
        CompiledExternalModule(CompiledModule module)
        {
            GLSLD_ASSERT(module.dependentModule == nullptr);
            diagContext = std::move(module.diagContext);
            lexContext  = std::move(module.lexContext);
            astContext  = std::move(module.astContext);
            symbolTable = std::move(module.symbolTable);
        }
    };

    class GlslCompiler final
    {
    public:
        auto CompileModule(std::string_view sourceString, std::shared_ptr<CompiledExternalModule> module = nullptr)
            -> std::shared_ptr<CompiledModule>
        {
            return std::make_shared<CompiledModule>(CompileImpl(sourceString, module));
        }

        auto CompileExternalModule(std::string_view sourceString) -> std::shared_ptr<CompiledExternalModule>
        {
            return std::make_shared<CompiledExternalModule>(CompileImpl(sourceString, nullptr));
        }

    private:
        auto CompileImpl(std::string_view sourceString, std::shared_ptr<CompiledExternalModule> module)
            -> CompiledModule
        {
            CompiledModule result;
            result.diagContext = std::make_unique<DiagnosticContext>();
            result.lexContext  = std::make_unique<LexContext>(sourceString);
            result.astContext  = std::make_unique<AstContext>();

            Parser parser{result.lexContext.get(), result.astContext.get(), result.diagContext.get()};
            parser.DoParseTranslationUnit();

            result.symbolTable =
                TypeChecker{}.TypeCheck(*result.astContext, module ? &module->GetSymbolTable() : nullptr);

#if defined(GLSLD_DEBUG)
            AstPrinter printer;
            printer.TraverseAst(*result.astContext);
            fmt::print(stderr, "{}", printer.Export());
#endif

            return result;
        }
    };

    template <typename Derived>
    class ModuleVisitor : protected AstVisitor<Derived>
    {
    public:
        ModuleVisitor(CompiledModule& module) : module(&module)
        {
        }

        auto Traverse() -> void
        {
            AstVisitor<Derived>::TraverseAst(module->GetAstContext());
        }

    private:
        CompiledModule* module;
    };

} // namespace glsld