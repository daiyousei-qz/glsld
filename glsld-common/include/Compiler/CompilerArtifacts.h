#pragma once

#include "Ast/Misc.h"
#include "Compiler/SyntaxToken.h"
#include <algorithm>

namespace glsld
{
    // Tokens parsed from a translation unit. This may not own the token memory but just a view to that of the preamble.
    class LexedTranslationUnit
    {
    private:
        TranslationUnitID id;
        std::vector<RawSyntaxTokenEntry> syntaxTokenBuffer;
        std::vector<RawCommentTokenEntry> commentTokenBuffer;
        ArrayView<RawSyntaxTokenEntry> tokens;
        ArrayView<RawCommentTokenEntry> comments;

    public:
        LexedTranslationUnit(TranslationUnitID id, std::vector<RawSyntaxTokenEntry> tokens,
                             std::vector<RawCommentTokenEntry> comments)
            : id(id), syntaxTokenBuffer(std::move(tokens)), commentTokenBuffer(std::move(comments))
        {
            this->tokens   = this->syntaxTokenBuffer;
            this->comments = this->commentTokenBuffer;
            GLSLD_ASSERT(!this->tokens.empty());
        }
        LexedTranslationUnit(TranslationUnitID id, ArrayView<RawSyntaxTokenEntry> tokens,
                             ArrayView<RawCommentTokenEntry> comments)
            : id(id), tokens(tokens), comments(comments)
        {
            GLSLD_ASSERT(!tokens.empty());
        }

        LexedTranslationUnit(const LexedTranslationUnit&)                    = delete;
        auto operator=(const LexedTranslationUnit&) -> LexedTranslationUnit& = delete;
        LexedTranslationUnit(LexedTranslationUnit&&)                         = default;
        auto operator=(LexedTranslationUnit&&) -> LexedTranslationUnit&      = default;

        auto GetID() const noexcept -> TranslationUnitID
        {
            return id;
        }
        auto GetTokens() const noexcept -> ArrayView<RawSyntaxTokenEntry>
        {
            return tokens;
        }
        auto GetComments() const noexcept -> ArrayView<RawCommentTokenEntry>
        {
            return comments;
        }

        auto CreateSyntaxToken(uint32_t tokIndex) const noexcept -> SyntaxToken
        {
            GLSLD_ASSERT(tokIndex < tokens.size());
            return SyntaxToken{
                .index = SyntaxTokenID{static_cast<uint32_t>(id), tokIndex},
                .klass = tokens[tokIndex].klass,
                .text  = tokens[tokIndex].text,
            };
        }
    };

    class CompilerArtifacts
    {
    private:
        // We have maximum 3 translation unit for one compile right now. They are:
        // - System preamble
        // - User preamble
        // - User file
        static constexpr size_t NumTranslationUnit = 3;

        std::array<std::optional<LexedTranslationUnit>, NumTranslationUnit> lexInfo;
        std::array<const AstTranslationUnit*, NumTranslationUnit> astInfo;

    public:
        auto GetLexedTranslationUnit(TranslationUnitID id) const -> const LexedTranslationUnit*
        {
            const auto& info = lexInfo[static_cast<size_t>(id)];
            if (!info.has_value()) {
                return nullptr;
            }

            return &*info;
        }

        auto GetSyntaxTokens(TranslationUnitID id) const -> ArrayView<RawSyntaxTokenEntry>
        {
            const auto& info = lexInfo[static_cast<size_t>(id)];
            if (!info.has_value()) {
                return {};
            }

            return info->GetTokens();
        }

        auto GetCommentTokens(TranslationUnitID id) const -> ArrayView<RawCommentTokenEntry>
        {
            const auto& info = lexInfo[static_cast<size_t>(id)];
            if (!info.has_value()) {
                return {};
            }

            return info->GetComments();
        }

        auto GetAst(TranslationUnitID id) const -> const AstTranslationUnit*
        {
            return astInfo[static_cast<size_t>(id)];
        }

        auto UpdateLexInfo(LexedTranslationUnit tu) -> void
        {
            lexInfo[static_cast<size_t>(tu.GetID())].emplace(std::move(tu));
        }

        auto UpdateAst(const AstTranslationUnit* ast) -> void
        {
            astInfo[static_cast<size_t>(ast->GetSyntaxRange().GetTranslationUnit())] = ast;
        }
    };
} // namespace glsld