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
    struct CompilerFeatureConfig
    {
        bool enableInt8Type             = false;
        bool enableInt16Type            = false;
        bool enableInt32Type            = false;
        bool enableInt64Type            = false;
        bool enableFloat16Type          = false;
        bool enableFloat32Type          = false;
        bool enableFloat64Type          = false;
        bool enableMemoryScopeSemantics = false;
        bool enableRayQuery             = false;
        bool enableRayTracingEXT        = false;
        bool enableRayTracingNV         = false;
    };

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

        std::unique_ptr<CompilerArtifact> systemPreambleArtifacts;
        std::unique_ptr<CompilerArtifact> userPreambleArtifacts;
        std::unique_ptr<CompilerArtifact> userFileArtifacts;

#if defined(GLSLD_DEBUG)
        mutable CompilerTrace trace;
#endif

        auto Initialize() -> void;
        auto InitializeStdlib() -> void;

        auto TryDumpTokens(TranslationUnitID id, ArrayView<RawSyntaxToken> tokens) const -> void;
        auto TryDumpAst(TranslationUnitID id, const AstTranslationUnit* ast) const -> void;

    public:
        CompilerInvocationState(SourceManager& sourceManager, CompilerConfig compilerConfig,
                                std::shared_ptr<PrecompiledPreamble> preamble)
            : sourceManager(sourceManager), compilerConfig(compilerConfig),
              languageConfig(preamble->GetLanguageConfig()), preamble(std::move(preamble))
        {
            GLSLD_ASSERT(this->preamble != nullptr);
            Initialize();
            // stdlib imported from preamble if any.
            // FIXME: import macros from preamble
        }
        CompilerInvocationState(SourceManager& sourceManager, CompilerConfig compilerConfig,
                                LanguageConfig languageConfig)
            : sourceManager(sourceManager), compilerConfig(compilerConfig), languageConfig(languageConfig)
        {
            Initialize();
            InitializeStdlib();
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

        auto GetArtifact(TranslationUnitID id) noexcept -> CompilerArtifact*
        {
            return id == TranslationUnitID::SystemPreamble ? systemPreambleArtifacts.get()
                   : id == TranslationUnitID::UserPreamble ? userPreambleArtifacts.get()
                                                           : userFileArtifacts.get();
        }

        auto UpdatePreprocessingArtifact(TranslationUnitID id, std::vector<RawSyntaxToken> tokens,
                                         std::vector<RawCommentToken> comments, std::vector<PreprocessedFile> files)
            -> void
        {
            TryDumpTokens(id, tokens);
            GetArtifact(id)->UpdatePreprocessingArtifact(std::move(tokens), std::move(comments), std::move(files));
        }

        auto UpdateAstArtifact(TranslationUnitID id, const AstTranslationUnit* ast) -> void
        {
            TryDumpAst(id, ast);
            GetArtifact(id)->UpdateAstArtifact(ast);
        }

        auto CreatePreamble() noexcept -> std::shared_ptr<PrecompiledPreamble>
        {
            return std::make_shared<PrecompiledPreamble>(
                languageConfig, sourceManager.GetSystemPreamble(), sourceManager.GetUserPreamble(),
                std::move(atomTable), std::move(macroTable), std::move(symbolTable), std::move(astContext),
                std::move(systemPreambleArtifacts), std::move(userPreambleArtifacts));
        }
        auto CreateCompileResult() noexcept -> std::unique_ptr<CompilerResult>
        {
            return std::make_unique<CompilerResult>(std::move(preamble), std::move(atomTable), std::move(astContext),
                                                    std::move(systemPreambleArtifacts),
                                                    std::move(userPreambleArtifacts), std::move(userFileArtifacts));
        }
    };
} // namespace glsld