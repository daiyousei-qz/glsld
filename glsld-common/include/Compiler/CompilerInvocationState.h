#pragma once
#include "Ast/Misc.h"
#include "Basic/AtomTable.h"
#include "Basic/Common.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerArtifacts.h"
#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerResult.h"
#include "Compiler/CompilerTrace.h"
#include "Compiler/DiagnosticStream.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SourceManager.h"
#include "Compiler/SymbolTable.h"

#include <memory>

namespace glsld
{
    class CompilerInvocationState
    {
    private:
        SourceManager& sourceManager;
        CompilerConfig compilerConfig;
        LanguageConfig languageConfig;
        std::shared_ptr<PrecompiledPreamble> preamble = nullptr;

        std::unique_ptr<AtomTable> atomTable;
        std::unique_ptr<MacroTable> macroTable;
        std::unique_ptr<SymbolTable> symbolTable;
        std::unique_ptr<AstContext> astContext;
        std::unique_ptr<DiagnosticStream> diagStream;

        std::unique_ptr<CompilerArtifacts> artifacts;

#if defined(GLSLD_DEBUG)
        mutable CompilerTrace trace;
#endif

        auto Initialize() -> void;

    public:
        CompilerInvocationState(SourceManager& sourceManager, CompilerConfig compilerConfig,
                                std::shared_ptr<PrecompiledPreamble> preamble)
            : sourceManager(sourceManager), compilerConfig(compilerConfig),
              languageConfig(preamble->GetLanguageConfig()), preamble(std::move(preamble))
        {
            GLSLD_ASSERT(this->preamble != nullptr);
            Initialize();
        }
        CompilerInvocationState(SourceManager& sourceManager, CompilerConfig compilerConfig,
                                LanguageConfig languageConfig)
            : sourceManager(sourceManager), compilerConfig(compilerConfig), languageConfig(languageConfig)
        {
            Initialize();
        }

        auto GetSourceManager() noexcept -> SourceManager&
        {
            return sourceManager;
        }
        auto GetCompilerConfig() const noexcept -> const CompilerConfig&
        {
            return compilerConfig;
        }
        auto GetLanguageConfig() const noexcept -> const LanguageConfig&
        {
            return languageConfig;
        }

        auto GetAtomTable() noexcept -> AtomTable&
        {
            return *atomTable;
        }
        auto GetMacroTable() noexcept -> MacroTable&
        {
            return *macroTable;
        }
        auto GetSymbolTable() noexcept -> SymbolTable&
        {
            return *symbolTable;
        }
        auto GetAstContext() noexcept -> AstContext&
        {
            return *astContext;
        }
        auto GetDiagnosticStream() noexcept -> DiagnosticStream&
        {
            return *diagStream;
        }

#if defined(GLSLD_DEBUG)
        auto GetCompilerTrace() const noexcept -> CompilerTrace&
        {
            return trace;
        }
#endif

        auto GetLexedTranslationUnit(TranslationUnitID id) const -> const LexedTranslationUnit*
        {
            return artifacts->GetLexedTranslationUnit(id);
        }
        auto SetLexedTranslationUnit(LexedTranslationUnit tu) -> void
        {
            if (compilerConfig.dumpTokens) {
                if (tu.GetID() == TranslationUnitID::UserPreamble && tu.GetTokens().size() > 1) {
                    Print("=====Tokens of User Preamble=====\n");
                    auto index = 0;
                    for (const auto& token : tu.GetTokens()) {
                        const auto& expanedRange = token.expandedRange;
                        Print("{}: [{}]'{}' @ ({},{}~{},{})\n", index++, TokenKlassToString(token.klass),
                              token.text.StrView(), expanedRange.start.line, expanedRange.start.character,
                              expanedRange.end.line, expanedRange.end.character);
                    }
                }
                else if (tu.GetID() == TranslationUnitID::UserFile) {
                    Print("=====Tokens of User File=====\n");
                    auto index = 0;
                    for (const auto& token : tu.GetTokens()) {
                        const auto& expanedRange = token.expandedRange;
                        Print("{}: [{}]'{}' @ ({},{}~{},{})\n", index++, TokenKlassToString(token.klass),
                              token.text.StrView(), expanedRange.start.line, expanedRange.start.character,
                              expanedRange.end.line, expanedRange.end.character);
                    }
                }
            }

            artifacts->UpdateLexInfo(std::move(tu));
        }
        auto GetAstTranslationUnit(TranslationUnitID id) const -> const AstTranslationUnit*
        {
            return artifacts->GetAst(id);
        }
        auto SetAstTranslationUnit(TranslationUnitID id, const AstTranslationUnit* ast) -> void
        {
            if (compilerConfig.dumpAst) {
                if (id == TranslationUnitID::UserPreamble && !ast->GetGlobalDecls().empty()) {
                    Print("=====AST of User Preamble=====\n");
                    Print("{}", ast->Print());
                }

                if (id == TranslationUnitID::UserFile) {
                    Print("=====AST of User File=====\n");
                    Print("{}", ast->Print());
                }
            }

            artifacts->UpdateAst(ast);
        }

        auto CreatePreamble() noexcept -> std::shared_ptr<PrecompiledPreamble>
        {
            return std::make_shared<PrecompiledPreamble>(languageConfig, sourceManager.GetSystemPreamble(),
                                                         sourceManager.GetUserPreamble(), std::move(atomTable),
                                                         std::move(macroTable), std::move(symbolTable),
                                                         std::move(astContext), std::move(artifacts));
        }
        auto CreateCompileResult() noexcept -> std::unique_ptr<CompilerResult>
        {
            return std::make_unique<CompilerResult>(std::move(atomTable), std::move(astContext), std::move(artifacts));
        }
    };
} // namespace glsld