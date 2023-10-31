#pragma once
#include "Basic/Common.h"
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
        bool skipUserPreamble = false;

        // The maximum number of nested include levels.
        int maxIncludeDepth = 16;

        // The include paths to search for included files.
        std::vector<std::filesystem::path> includePaths;
    };

    class CompiledPreamble
    {
    private:
        friend class CompilerObject;

        std::unique_ptr<const SourceContext> sourceContext;
        std::unique_ptr<const LexContext> lexContext;
        std::unique_ptr<const AstContext> astContext;
        std::unique_ptr<const SymbolTable> symbolTable;

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

        auto GetSymbolTable() const noexcept -> const SymbolTable&
        {
            return *symbolTable;
        }
    };

    class CompilerObject final
    {
    private:
        bool compiled;

        CompilerConfig config;

        std::shared_ptr<CompiledPreamble> preamble;

        std::unique_ptr<SourceContext> sourceContext;
        std::unique_ptr<DiagnosticStream> diagStream;
        std::unique_ptr<LexContext> lexContext;
        std::unique_ptr<AstContext> astContext;
        std::unique_ptr<SymbolTable> symbolTable;

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

        auto GetFileSystemProvider() noexcept -> FileSystemProvider&
        {
            return DefaultFileSystemProvider::GetInstance();
        }

        auto GetSourceContext() const noexcept -> const SourceContext&
        {
            return *sourceContext;
        }
        auto GetSourceContext() noexcept -> SourceContext&
        {
            return *sourceContext;
        }

        auto GetDiagnosticContext() const noexcept -> const DiagnosticStream&
        {
            return *diagStream;
        }
        auto GetDiagnosticContext() noexcept -> DiagnosticStream&
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

        auto CreatePreamble() -> std::shared_ptr<CompiledPreamble>;

        auto CompileFromFile(StringView path, std::shared_ptr<CompiledPreamble> preamble, PPCallback* ppCallback)
            -> void;

        auto CompileFromBuffer(StringView sourceText, std::shared_ptr<CompiledPreamble> preamble,
                               PPCallback* ppCallback) -> void;

    private:
        auto InitializeCompilation(std::shared_ptr<CompiledPreamble> preamble) -> void;
        auto DoCompile(PPCallback* callback) -> void;
        auto FinalizeCompilation() -> void;
    };

    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>;

} // namespace glsld