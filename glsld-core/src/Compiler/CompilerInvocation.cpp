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

    auto CompilerInvocation::AddIncludeUri(ParsedUri uri) -> void
    {
        // FIXME: "file:/tmp/.." would be simplified into "file:/tmp/", which is incorrect.
        compilerConfig.includeUris.push_back(uri.RemoveFileName().ToUri());
    }

    auto CompilerInvocation::SetMainFileFromUri(ParsedUri uri) -> void
    {
        // FIXME: report error
        mainFileId = sourceManager.OpenFromUri(uri);
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

        CompilerInvocationState compiler{sourceManager, compilerConfig, languageConfig, false};
        Preprocessor{compiler, mainFileId, ppCallback, true}.DoPreprocess();
    }

    auto CompilerInvocation::CompilePreamble(PPCallback* ppCallback) -> std::shared_ptr<PrecompiledPreamble>
    {
        GLSLD_REQUIRE(!preamble);

        ScopeExit _{[this, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
        }};

        auto compiler = std::make_unique<CompilerInvocationState>(sourceManager, compilerConfig, languageConfig, true);
        if (!languageConfig.noStdlib) {
            DoPreprocess(*compiler, FileID::SystemPreamble(), nullptr);
            DoParse(*compiler);
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

        std::shared_ptr<PrecompiledPreamble> usedPreamble = preamble;
        if (!usedPreamble && !languageConfig.noStdlib) {
            auto preambleCompiler =
                std::make_unique<CompilerInvocationState>(sourceManager, compilerConfig, languageConfig, true);
            DoPreprocess(*preambleCompiler, FileID::SystemPreamble(), nullptr);
            DoParse(*preambleCompiler);
            usedPreamble = preambleCompiler->CreatePreamble();
        }

        std::unique_ptr<CompilerInvocationState> compiler;
        if (usedPreamble) {
            compiler = std::make_unique<CompilerInvocationState>(sourceManager, compilerConfig, usedPreamble);
        }
        else {
            compiler = std::make_unique<CompilerInvocationState>(sourceManager, compilerConfig, languageConfig, false);
        }

        DoPreprocess(*compiler, mainFileId, ppCallback);
        if (mode == CompileMode::PreprocessOnly) {
            return compiler->CreateCompileResult();
        }

        DoParse(*compiler);

        return compiler->CreateCompileResult();
    }

    auto CompilerInvocation::DoPreprocess(CompilerInvocationState& compiler, FileID file, PPCallback* callback) -> void
    {
        GLSLD_REQUIRE(file.IsValid());

        ScopeExit _{[this, file, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.mainFileLexing += elapsedTime;
        }};

        Preprocessor{compiler, file, callback, false}.DoPreprocess();
    }
    auto CompilerInvocation::DoParse(CompilerInvocationState& compiler) -> void
    {
        ScopeExit _{[this, timer = SimpleTimer{}]() {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.mainFileParsing += elapsedTime;
        }};

        Parser{compiler}.DoParse();
    }

} // namespace glsld
