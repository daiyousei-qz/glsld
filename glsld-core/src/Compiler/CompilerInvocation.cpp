#include "Basic/Common.h"
#include "Support/ScopeExit.h"
#include "Support/SimpleTimer.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/CompilerInvocationState.h"
#include "Compiler/CompilerResult.h"
#include "Compiler/Preprocessor.h"
#include "Compiler/Parser.h"
#include "Compiler/SyntaxToken.h"

#include <memory>

namespace glsld
{
    CompilerInvocation::CompilerInvocation() = default;
    CompilerInvocation::CompilerInvocation(std::shared_ptr<PrecompiledPreamble> preamble)
    {
        this->languageConfig = preamble->GetLanguageConfig();
        this->preamble       = std::move(preamble);
    }

    auto CompilerInvocation::SetMainFileFromFile(StringView path) -> void
    {
        // FIXME: report error
        mainFileId = sourceManager.OpenFromFile(path.StdStrView());
    }

    auto CompilerInvocation::SetMainFileFromBuffer(StringView sourceText) -> void
    {
        mainFileId = sourceManager.OpenFromBuffer(sourceText);
    }

    auto CompilerInvocation::ScanVersionAndExtension(PPCallback* ppCallback) -> void
    {
        if (!mainFileId.IsValid()) {
            // FIXME: report error
            return;
        }

        ScopeExit _{[this, timer = SimpleTimer{}] {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
            statistics.versionScanning += elapsedTime;
        }};

        CompilerInvocationState compiler{sourceManager, compilerConfig, languageConfig};
        Preprocessor{compiler, mainFileId, ppCallback, true}.DoPreprocess();
    }

    auto CompilerInvocation::CompilePreamble(PPCallback* ppCallback) -> std::shared_ptr<PrecompiledPreamble>
    {
        GLSLD_REQUIRE(!preamble);

        ScopeExit _{[this, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
        }};

        auto compiler = InitializeCompilation();

        if (compiler->GetArtifact(TranslationUnitID::SystemPreamble)->GetAst() == nullptr) {
            DoPreprocess(*compiler, FileID::SystemPreamble(), nullptr);
            DoParse(*compiler, TranslationUnitID::SystemPreamble);
        }

        if (compiler->GetArtifact(TranslationUnitID::UserPreamble)->GetAst() == nullptr) {
            DoPreprocess(*compiler, FileID::UserPreamble(), ppCallback);
            DoParse(*compiler, TranslationUnitID::UserPreamble);
        }

        return compiler->CreatePreamble();
    }

    auto CompilerInvocation::CompileMainFile(PPCallback* ppCallback, CompileMode mode)
        -> std::unique_ptr<CompilerResult>
    {
        if (!mainFileId.IsValid()) {
            // FIXME: report error
            return nullptr;
        }

        ScopeExit _{[this, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
        }};

        auto compiler = InitializeCompilation();

        if (!preamble) {
            DoPreprocess(*compiler, FileID::SystemPreamble(), nullptr);
            DoPreprocess(*compiler, FileID::UserPreamble(), ppCallback);
        }

        DoPreprocess(*compiler, mainFileId, ppCallback);
        if (mode == CompileMode::PreprocessOnly) {
            return compiler->CreateCompileResult();
        }

        if (!preamble) {
            DoParse(*compiler, TranslationUnitID::SystemPreamble);
            DoParse(*compiler, TranslationUnitID::UserPreamble);
        }
        DoParse(*compiler, TranslationUnitID::UserFile);

        return compiler->CreateCompileResult();
    }

    auto CompilerInvocation::InitializeCompilation() -> std::unique_ptr<CompilerInvocationState>
    {
        if (preamble) {
            return std::make_unique<CompilerInvocationState>(sourceManager, compilerConfig, preamble);
        }
        else {
            return std::make_unique<CompilerInvocationState>(sourceManager, compilerConfig, languageConfig);
        }
    }

    auto CompilerInvocation::DoPreprocess(CompilerInvocationState& compiler, FileID file, PPCallback* callback) -> void
    {
        GLSLD_REQUIRE(file.IsValid());

        ScopeExit _{[this, file, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            if (file.IsPreamble()) {
                statistics.preambleLexing += elapsedTime;
            }
            else {
                statistics.mainFileLexing += elapsedTime;
            }
        }};

        Preprocessor{compiler, file, callback, false}.DoPreprocess();
    }
    auto CompilerInvocation::DoParse(CompilerInvocationState& compiler, TranslationUnitID id) -> void
    {
        ScopeExit _{[this, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.mainFileParsing += elapsedTime;
        }};

        Parser{compiler, id, compiler.GetArtifact(id)->GetTokens()}.DoParse();
    }

} // namespace glsld