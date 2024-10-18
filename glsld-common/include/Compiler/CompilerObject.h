#pragma once
#include "Basic/Common.h"
#include "Basic/SourceInfo.h"
#include "Basic/StringView.h"
#include "Basic/FileSystemProvider.h"

#include <memory>
#include <vector>
#include <filesystem>

namespace glsld
{
    class PPCallback;

    class SourceContext;
    class DiagnosticStream;
    class LexContext;
    class AstContext;
    class SymbolTable;

    struct CompilerConfig
    {
        // Dumps the token stream in stdout.
        bool dumpTokens = false;

        // Dumps the parsed AST in stdout.
        bool dumpAst = false;

        // Whether the compiler should skip tokens in the preamble.
        // The user preamble is defined as all tokens before any non-comment valid tokens in the main file.
        // bool skipUserPreamble = false;

        // The maximum number of nested include levels.
        int maxIncludeDepth = 32;

        // The include paths to search for included files.
        std::vector<std::filesystem::path> includePaths;

        // The preprocessor definitions.
        std::vector<std::string> defines;
    };

    class CompiledPreamble
    {
    private:
        friend class CompilerObject;

        std::vector<char> systemPreambleContent;
        std::vector<char> userPreambleContent;

        std::unique_ptr<const SourceContext> sourceContext;
        std::unique_ptr<const LexContext> lexContext;
        std::unique_ptr<const AstContext> astContext;

    public:
        CompiledPreamble();
        ~CompiledPreamble();

        auto GetSourceContext() const noexcept -> const SourceContext&
        {
            return *sourceContext;
        }

        auto GetLexContext() const noexcept -> const LexContext&
        {
            return *lexContext;
        }

        auto GetAstContext() const noexcept -> const AstContext&
        {
            return *astContext;
        }
    };

    class CompilerObject final
    {
    private:
        bool compiled = false;

        CompilerConfig config = {};

        std::shared_ptr<CompiledPreamble> preamble = nullptr;

        // Only effective when preamble is nullptr
        StringView systemPreambleContent = "";

        // Only effective when preamble is nullptr
        StringView userPreambleContent = "";

        std::unique_ptr<SourceContext> sourceContext = nullptr;
        std::unique_ptr<LexContext> lexContext       = nullptr;
        std::unique_ptr<AstContext> astContext       = nullptr;

        std::unique_ptr<DiagnosticStream> diagStream = nullptr;

    public:
        CompilerObject();
        ~CompilerObject();

        CompilerObject(const CompilerObject&)                    = delete;
        auto operator=(const CompilerObject&) -> CompilerObject& = delete;
        CompilerObject(CompilerObject&&)                         = delete;
        auto operator=(CompilerObject&&) -> CompilerObject&      = delete;

        auto IsCompiled() const -> bool
        {
            return compiled;
        }

        auto GetConfig() const noexcept -> const CompilerConfig&
        {
            return config;
        }

        auto GetSourceContext() const noexcept -> const SourceContext&
        {
            return *sourceContext;
        }
        auto GetSourceContext() noexcept -> SourceContext&
        {
            return *sourceContext;
        }

        auto GetDiagnosticStream() const noexcept -> const DiagnosticStream&
        {
            return *diagStream;
        }
        auto GetDiagnosticStream() noexcept -> DiagnosticStream&
        {
            return *diagStream;
        }

        auto GetLexContext() const noexcept -> const LexContext&
        {
            return *lexContext;
        }
        auto GetLexContext() noexcept -> LexContext&
        {
            return *lexContext;
        }

        auto GetAstContext() const noexcept -> const AstContext&
        {
            return *astContext;
        }
        auto GetAstContext() noexcept -> AstContext&
        {
            return *astContext;
        }

        auto Reset() -> void;

        auto SetDumpTokens(bool value) -> void
        {
            config.dumpTokens = value;
        }

        auto SetDumpAst(bool value) -> void
        {
            config.dumpAst = value;
        }

        auto AddIncludePath(const std::filesystem::path& path) -> void
        {
            // FIXME: Check if path is valid
            config.includePaths.push_back(path);
        }

        auto SetSystemPreamble(StringView content) -> void
        {
            systemPreambleContent = content;
        }

        auto SetUserPreamble(StringView content) -> void
        {
            userPreambleContent = content;
        }

        auto SetPrecompiledPreamble(std::shared_ptr<CompiledPreamble> preamble) -> void
        {
            this->preamble = std::move(preamble);
        }

        auto CompilePreamble() -> std::shared_ptr<CompiledPreamble>;

        auto CompileFromFile(StringView path, PPCallback* ppCallback) -> void;

        auto CompileFromBuffer(StringView sourceText, PPCallback* ppCallback) -> void;

    private:
        auto InitializeCompilation() -> void;
        auto DoPreprocess(FileID file, PPCallback* callback) -> void;
        auto DoParse() -> void;
        auto DoTypeCheck() -> void;
        auto FinalizeCompilation() -> void;
    };

    // 1. Compile with text preambles
    // 2. Compile with pre-compiled preamble

    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>;

} // namespace glsld