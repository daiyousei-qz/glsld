#pragma once
#include "Basic/Common.h"
#include "Basic/SourceInfo.h"
#include "Basic/StringView.h"
#include "Compiler/CompilerInvocationState.h"
#include "Language/ShaderTarget.h"
#include "Language/Extension.h"
#include "Compiler/SourceManager.h"
#include "Compiler/CompilerConfig.h"
#include "Compiler/CompilerResult.h"

#include <chrono>
#include <memory>
#include <vector>
#include <filesystem>

namespace glsld
{
    class PPCallback;

    struct CompilerInvocationStatistics
    {
        using Duration = std::chrono::nanoseconds;
        Duration totalCompileTime;

        // Time spent on scanning #version and #extension
        Duration versionScanning;

        // Time spent on lexing preamble
        Duration preambleLexing;

        // Time spent on lexing main file
        Duration mainFileLexing;

        // Time spent on parsing
        Duration mainFileParsing;
    };

    class CompilerInvocation final
    {
    private:
        std::shared_ptr<PrecompiledPreamble> preamble = nullptr;

        CompilerConfig compilerConfig = {};

        // Only effective when preamble is nullptr
        LanguageConfig languageConfig = {};

        FileID mainFileId = {};

        SourceManager sourceManager;

        CompilerInvocationStatistics statistics;

    public:
        CompilerInvocation();
        CompilerInvocation(std::shared_ptr<PrecompiledPreamble> preamble);

        CompilerInvocation(const CompilerInvocation&)                    = delete;
        auto operator=(const CompilerInvocation&) -> CompilerInvocation& = delete;
        CompilerInvocation(CompilerInvocation&&)                         = delete;
        auto operator=(CompilerInvocation&&) -> CompilerInvocation&      = delete;

        auto GetCompilerConfig() const noexcept -> const CompilerConfig&
        {
            return compilerConfig;
        }

        auto GetLanguageConfig() const noexcept -> const LanguageConfig&
        {
            return languageConfig;
        }

        auto GetStatistics() const noexcept -> CompilerInvocationStatistics
        {
            return statistics;
        }

        auto SetDumpTokens(bool value) -> void
        {
            compilerConfig.dumpTokens = value;
        }

        auto SetDumpAst(bool value) -> void
        {
            compilerConfig.dumpAst = value;
        }

        auto SetCountUtf16Characters(bool value) -> void
        {
            compilerConfig.countUtf16Character = value;
        }

        auto AddIncludePath(const std::filesystem::path& path) -> void
        {
            // FIXME: Check if path is valid
            compilerConfig.includePaths.push_back(path);
        }

        auto SetGlslVersion(GlslVersion version, GlslProfile profile) -> void
        {
            if (!preamble) {
                languageConfig.version = version;
                languageConfig.profile = profile;
            }
        }

        auto SetShaderStage(GlslShaderStage stage) -> void
        {
            if (!preamble) {
                languageConfig.stage = stage;
            }
        }

        auto EnableExtension(ExtensionId id) -> void
        {
            if (!preamble) {
                languageConfig.extensions.EnableExtension(id);
            }
        }

        auto SetNoStdlib() -> void
        {
            if (!preamble) {
                languageConfig.noStdlib = true;
            }
        }

        // User should ensure that the preamble text outlive the CompilerInvocation
        auto SetUserPreamble(StringView content) -> void
        {
            GLSLD_REQUIRE(preamble == nullptr);
            sourceManager.SetUserPreamble(content);
        }

        auto SetMainFileFromFile(StringView path) -> void;

        // User should ensure that the source text outlive the CompilerInvocation
        auto SetMainFileFromBuffer(StringView sourceText) -> void;

        // Scan the starting part of the main file to get the version and extensions.
        // Scanning should end at the first non-comment, non-preprocessor token.
        auto ScanVersionAndExtension(PPCallback* ppCallback) -> void;

        // Compile the system preamble, user preamble and optionally main file into a precompiled preamble for reuse.
        auto CompilePreamble(PPCallback* ppCallback) -> std::shared_ptr<PrecompiledPreamble>;

        auto CompileMainFile(PPCallback* ppCallback) -> std::unique_ptr<CompilerResult>;

    private:
        auto InitializeCompilation() -> std::unique_ptr<CompilerInvocationState>;
        auto DoPreprocess(CompilerInvocationState& compiler, FileID file, PPCallback* callback) -> void;
        auto DoParse(CompilerInvocationState& compiler, const LexedTranslationUnit& tu) -> void;
    };

} // namespace glsld