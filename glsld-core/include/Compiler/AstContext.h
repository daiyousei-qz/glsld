#pragma once
#include "Ast/Misc.h"
#include "Support/MemoryArena.h"
#include "Ast/Base.h"

#include <map>

namespace glsld
{
    struct PreprocessedTokens
    {
        std::vector<RawSyntaxToken> tokens;
        std::vector<RawCommentToken> comments;
        std::vector<PreprocessedFile> files;
    };

    // This class manages everything related to parsing of a translation unit, primarily AST.
    class AstContext final
    {
    private:
        // The cached array types.
        std::map<std::pair<const Type*, size_t>, const Type*> arrayTypeCache;

        // The memory arena that holds all memory allocated for AST.
        MemoryArena arena;

        std::vector<RawSyntaxToken> tokens;
        std::vector<RawCommentToken> comments;
        std::vector<PreprocessedFile> files;

        const AstTranslationUnit* ast = nullptr;

    public:
        // FIXME: check how struct/array type in the preamble is handled
        AstContext() = default;

        auto GetArena() noexcept -> MemoryArena&
        {
            return arena;
        }

        auto AddToken(RawSyntaxToken token) -> void
        {
            tokens.push_back(token);
        }

        auto AddCommentToken(RawCommentToken token) -> void
        {
            comments.push_back(token);
        }

        auto AddPreprocessedFile(PreprocessedFile file) -> void
        {
            files.push_back(file);
        }

        auto GetNextTokenIndex() const noexcept -> uint32_t
        {
            return static_cast<uint32_t>(tokens.size());
        }

        auto GetNextCommentIndex() const noexcept -> uint32_t
        {
            return static_cast<uint32_t>(comments.size());
        }

        auto GetTokens() const -> ArrayView<RawSyntaxToken>
        {
            return tokens;
        }

        auto GetComments() const -> ArrayView<RawCommentToken>
        {
            return comments;
        }

        auto GetFiles() const -> ArrayView<PreprocessedFile>
        {
            return files;
        }

        auto CreateStructType(AstStructDecl& decl) -> const Type*;

        auto CreateInterfaceBlockType(AstInterfaceBlockDecl& decl) -> const Type*;

        auto GetArrayType(const Type* elementType, const AstArraySpec* arraySpec) -> const Type*;

        // TODO: To ensure Type of the same type always uses a single pointer, we need a common context for
        // different module compile?
        auto GetArrayType(const Type* elementType, size_t dimSizes) -> const Type*;

        auto SetAst(const AstTranslationUnit& ast) -> void
        {
            this->ast = &ast;
        }

        auto GetAst() const -> const AstTranslationUnit*
        {
            return ast;
        }
    };
} // namespace glsld