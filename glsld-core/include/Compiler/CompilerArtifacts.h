#pragma once

#include "Ast/Misc.h"
#include "Compiler/SyntaxToken.h"
#include <memory>

namespace glsld
{
    class CompilerArtifact
    {
    private:
        TranslationUnitID id;

        std::vector<RawSyntaxToken> syntaxTokenBuffer;
        std::vector<RawCommentToken> commentTokenBuffer;
        ArrayView<RawSyntaxToken> tokens;
        ArrayView<RawCommentToken> comments;

        const AstTranslationUnit* ast = nullptr;

    public:
        CompilerArtifact(TranslationUnitID id) : id(id)
        {
        }

        auto UpdateTokenArtifact(std::vector<RawSyntaxToken> lexedTokens, std::vector<RawCommentToken> lexedComments)
            -> void
        {
            GLSLD_ASSERT(tokens.size() == 0 && comments.size() == 0);
            syntaxTokenBuffer  = std::move(lexedTokens);
            commentTokenBuffer = std::move(lexedComments);
            tokens             = syntaxTokenBuffer;
            comments           = commentTokenBuffer;
        }

        auto UpdateAstArtifact(const AstTranslationUnit* parsedAst) -> void
        {
            GLSLD_ASSERT(ast == nullptr);
            ast = parsedAst;
        }

        auto CreateReference() const noexcept -> std::unique_ptr<CompilerArtifact>
        {
            auto artifact      = std::make_unique<CompilerArtifact>(id);
            artifact->tokens   = tokens;
            artifact->comments = comments;
            artifact->ast      = ast;

            return artifact;
        }

        auto GetID() const noexcept -> TranslationUnitID
        {
            return id;
        }
        auto GetTokens() const noexcept -> ArrayView<RawSyntaxToken>
        {
            return tokens;
        }
        auto GetComments() const noexcept -> ArrayView<RawCommentToken>
        {
            return comments;
        }
        auto GetAst() const noexcept -> const AstTranslationUnit*
        {
            return ast;
        }
    };
} // namespace glsld