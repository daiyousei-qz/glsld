#include "Basic/Common.h"
#include "Basic/SimpleTimer.h"
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

        SimpleTimer timer([this](SimpleTimer& timer) {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
            statistics.versionScanning += elapsedTime;
        });

        CompilerInvocationState compiler{sourceManager, compilerConfig, languageConfig};
        Preprocessor{compiler, ppCallback, true}.DoPreprocess(mainFileId);
    }

    auto CompilerInvocation::CompilePreamble(PPCallback* ppCallback) -> std::shared_ptr<PrecompiledPreamble>
    {
        GLSLD_REQUIRE(!preamble);

        SimpleTimer timer([this](SimpleTimer& timer) {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
        });

        auto compiler = InitializeCompilation();

        if (compiler->GetAstTranslationUnit(TranslationUnitID::SystemPreamble) == nullptr) {
            DoPreprocess(*compiler, FileID::SystemPreamble(), nullptr);
            DoParse(*compiler, *compiler->GetLexedTranslationUnit(TranslationUnitID::SystemPreamble));
        }

        if (compiler->GetAstTranslationUnit(TranslationUnitID::UserPreamble) == nullptr) {
            DoPreprocess(*compiler, FileID::UserPreamble(), ppCallback);
            DoParse(*compiler, *compiler->GetLexedTranslationUnit(TranslationUnitID::UserPreamble));
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

        SimpleTimer timer([this](SimpleTimer& timer) {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.totalCompileTime += elapsedTime;
        });

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
            DoParse(*compiler, *compiler->GetLexedTranslationUnit(TranslationUnitID::SystemPreamble));
            DoParse(*compiler, *compiler->GetLexedTranslationUnit(TranslationUnitID::UserPreamble));
        }
        DoParse(*compiler, *compiler->GetLexedTranslationUnit(TranslationUnitID::UserFile));

        return compiler->CreateCompileResult();
    }

    auto CompilerInvocation::InitializeCompilation() -> std::unique_ptr<CompilerInvocationState>
    {
        // FIXME: We enable all extensions by default for now. Support properly scanning the source code for extension
#define DECL_EXTENSION(EXTENSION_NAME) EnableExtension(ExtensionId::EXTENSION_NAME);
#include "GlslExtension.inc"
#undef DECL_EXTENSION
        // Initialize system preamble
        if (!preamble && !languageConfig.noStdlib) {
            StringView systemPreamble =
#include "Language/Stdlib.Generated.h"
                ;
            sourceManager.SetSystemPreamble(systemPreamble);
        }

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

        SimpleTimer timer([this, file](SimpleTimer& timer) {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            if (file.IsPreamble()) {
                statistics.preambleLexing += elapsedTime;
            }
            else {
                statistics.mainFileLexing += elapsedTime;
            }
        });

        Preprocessor{compiler, callback, false}.DoPreprocess(file);
    }
    auto CompilerInvocation::DoParse(CompilerInvocationState& compiler, const LexedTranslationUnit& tu) -> void
    {
        SimpleTimer timer([this](SimpleTimer& timer) {
            auto elapsedTime = timer.GetElapsedTime<CompilerInvocationStatistics::Duration>();
            statistics.mainFileParsing += elapsedTime;
        });

        Parser{compiler, tu}.DoParse();
    }

} // namespace glsld