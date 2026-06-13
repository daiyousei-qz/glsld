#pragma once

#include "Ast/Misc.h"
#include "Compiler/SyntaxToken.h"
#include <memory>

namespace glsld
{
    class CompilerArtifact
    {
    private:
        std::vector<RawSyntaxToken> syntaxTokenBuffer;
        std::vector<RawCommentToken> commentTokenBuffer;
        std::vector<PreprocessedFile> preprocessedFiles;
        ArrayView<RawSyntaxToken> tokens;
        ArrayView<RawCommentToken> comments;

        const AstTranslationUnit* ast = nullptr;

    public:
        auto UpdatePreprocessingArtifact(std::vector<RawSyntaxToken> lexedTokens,
                                         std::vector<RawCommentToken> lexedComments,
                                         std::vector<PreprocessedFile> files) -> void
        {
            GLSLD_ASSERT(tokens.size() == 0 && comments.size() == 0);
            syntaxTokenBuffer  = std::move(lexedTokens);
            commentTokenBuffer = std::move(lexedComments);
            preprocessedFiles  = std::move(files);
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
            auto artifact      = std::make_unique<CompilerArtifact>();
            artifact->tokens   = tokens;
            artifact->comments = comments;
            artifact->ast      = ast;

            return artifact;
        }

        auto GetTokens() const noexcept -> ArrayView<RawSyntaxToken>
        {
            return tokens;
        }
        auto GetComments() const noexcept -> ArrayView<RawCommentToken>
        {
            return comments;
        }
        auto GetFiles() const noexcept -> ArrayView<PreprocessedFile>
        {
            return preprocessedFiles;
        }
        auto GetAst() const noexcept -> const AstTranslationUnit*
        {
            return ast;
        }
    };
} // namespace glsld
