#pragma once
#include "Basic/Common.h"
#include "Basic/FileSystemProvider.h"

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace glsld
{
    class PPCallback;

    class SourceContext;
    class DiagnosticContext;
    class LexContext;
    class PreprocessContext;
    class AstContext;
    class TypeContext;
    class SymbolTable;

    struct CompilerConfig
    {
        bool noStdLib = false;

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

        auto GetPreprocessContext() const noexcept -> const PreprocessContext&
        {
            return *ppContext;
        }

        auto GetAstContext() const noexcept -> const AstContext&
        {
            return *astContext;
        }
        auto GetTypeContext() const noexcept -> const TypeContext&
        {
            return *typeContext;
        }
        auto GetSymbolTable() const noexcept -> const SymbolTable&
        {
            return *symbolTable;
        }

    private:
        friend class CompilerObject;

        int moduleId;

        std::unique_ptr<const SourceContext> sourceContext;
        std::unique_ptr<const LexContext> lexContext;
        std::unique_ptr<const PreprocessContext> ppContext;
        std::unique_ptr<const AstContext> astContext;
        std::unique_ptr<const TypeContext> typeContext;
        std::unique_ptr<const SymbolTable> symbolTable;
    };

    class CompilerObject final
    {
    private:
        bool compiled;
        int moduleId;

        CompilerConfig config;

        std::shared_ptr<CompiledPreamble> preamble;

        std::unique_ptr<SourceContext> sourceContext;
        std::unique_ptr<DiagnosticContext> diagContext;
        std::unique_ptr<LexContext> lexContext;
        std::unique_ptr<PreprocessContext> ppContext;
        std::unique_ptr<AstContext> astContext;
        std::unique_ptr<TypeContext> typeContext;
        std::unique_ptr<SymbolTable> symbolTable;

    public:
        CompilerObject();
        ~CompilerObject();

        // A CompilerObject is move only
        CompilerObject(const CompilerObject&)                    = delete;
        auto operator=(const CompilerObject&) -> CompilerObject& = delete;
        CompilerObject(CompilerObject&&)                         = default;
        auto operator=(CompilerObject&&) -> CompilerObject&      = default;

        auto IsCompiled() const -> bool
        {
            return compiled;
        }

        auto GetId() const -> int
        {
            return moduleId;
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

        auto GetDiagnosticContext() const noexcept -> const DiagnosticContext&
        {
            return *diagContext;
        }
        auto GetDiagnosticContext() noexcept -> DiagnosticContext&
        {
            return *diagContext;
        }

        auto GetLexContext() const noexcept -> const LexContext&
        {
            return *lexContext;
        }
        auto GetLexContext() noexcept -> LexContext&
        {
            return *lexContext;
        }

        auto GetPreprocessContext() const noexcept -> const PreprocessContext&
        {
            return *ppContext;
        }
        auto GetPreprocessContext() noexcept -> PreprocessContext&
        {
            return *ppContext;
        }

        auto GetAstContext() const noexcept -> const AstContext&
        {
            return *astContext;
        }
        auto GetAstContext() noexcept -> AstContext&
        {
            return *astContext;
        }

        auto GetTypeContext() const noexcept -> const TypeContext&
        {
            return *typeContext;
        }
        auto GetTypeContext() noexcept -> TypeContext&
        {
            return *typeContext;
        }

        auto Reset() -> void;

        auto AddIncludePath(const std::filesystem::path& path) -> void
        {
            // FIXME: Check if path is valid
            config.includePaths.push_back(path);
        }

        auto CreatePreamble() -> std::shared_ptr<CompiledPreamble>;

        auto Compile(StringView sourceText, std::shared_ptr<CompiledPreamble> preamble, PPCallback* callback) -> void;

    private:
        auto InitializeCompilation(std::shared_ptr<CompiledPreamble> preamble) -> void;
        auto FinalizeCompilation() -> void;
    };

    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>;

} // namespace glsld