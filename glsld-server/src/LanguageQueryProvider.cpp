#include "LanguageQueryProvider.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    auto LanguageQueryProvider::LookupToken(SyntaxTokenID id) const -> const RawSyntaxToken*
    {
        const CompilerArtifact* artifacts = nullptr;
        switch (id.GetTU()) {
        case TranslationUnitID::SystemPreamble:
            artifacts = &compilerResult->GetSystemPreambleArtifacts();
            break;
        case TranslationUnitID::UserPreamble:
            artifacts = &compilerResult->GetUserPreambleArtifacts();
            break;
        case TranslationUnitID::UserFile:
            artifacts = &compilerResult->GetUserFileArtifacts();
            break;
        }

        return &artifacts->GetTokens()[id.GetTokenIndex()];
    }
    auto LanguageQueryProvider::LookupTokens(AstSyntaxRange range) const -> ArrayView<RawSyntaxToken>
    {
        return {LookupToken(range.GetBeginID()), LookupToken(range.GetEndID())};
    }
    auto LanguageQueryProvider::LookupDotTokenIndex(const AstExpr& expr) const -> std::optional<SyntaxTokenID>
    {
        GLSLD_ASSERT(expr.Is<AstFieldAccessExpr>() || expr.Is<AstSwizzleAccessExpr>());
        auto range = expr.GetSyntaxRange();
        if (!range.Empty() && LookupToken(range.GetBackID())->klass == TokenKlass::Dot) {
            // CASE 1: `a.b`
            return range.GetBackID();
        }
        else if (range.GetTokenCount() > 1 && LookupToken(range.GetBackID() - 1)->klass == TokenKlass::Dot) {
            // CASE 2: `a. `
            return range.GetBackID() - 1;
        }
        else {
            return std::nullopt;
        }
    }
    auto LanguageQueryProvider::LookupTokenByPosition(TextPosition position) const -> ArrayView<RawSyntaxToken>
    {
        auto tokens           = compilerResult->GetUserFileArtifacts().GetTokens();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            tokens, position, {}, [](const RawSyntaxToken& tok) { return tok.expandedRange.start; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryProvider::LookupTokenByLine(uint32_t lineNum) const -> ArrayView<RawSyntaxToken>
    {
        auto tokens           = compilerResult->GetUserFileArtifacts().GetTokens();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            tokens, lineNum, {}, [](const RawSyntaxToken& tok) { return tok.expandedRange.start.line; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryProvider::LookupPreceedingComment(SyntaxTokenID id) const -> ArrayView<RawCommentToken>
    {
        if (id.GetTU() != TranslationUnitID::UserFile) {
            return {};
        }

        auto comments         = compilerResult->GetUserFileArtifacts().GetComments();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            comments, id.GetTokenIndex(), {}, [](const RawCommentToken& token) { return token.nextTokenIndex; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryProvider::LookupSpelledFile(SyntaxTokenID id) const -> FileID
    {
        return LookupToken(id)->spelledFile;
    }
    auto LanguageQueryProvider::IsMainFile(FileID file) const -> bool
    {
        return file == compilerResult->GetUserFileArtifacts().GetTokens().back().spelledFile;
    }
    auto LanguageQueryProvider::IsSpelledInMainFile(SyntaxTokenID id) const -> bool
    {
        if (id.GetTU() != TranslationUnitID::UserFile) {
            return false;
        }

        return LookupSpelledFile(id) == compilerResult->GetUserFileArtifacts().GetTokens().back().spelledFile;
    }
    auto LanguageQueryProvider::LookupSpelledTextRange(SyntaxTokenID id) const -> FileTextRange
    {
        auto token = LookupToken(id);
        return FileTextRange{
            .fileID = token->spelledFile,
            .range  = token->spelledRange,
        };
    }
    auto LanguageQueryProvider::LookupSpelledTextRangeInMainFile(SyntaxTokenID id) const -> std::optional<TextRange>
    {
        if (!IsSpelledInMainFile(id)) {
            return std::nullopt;
        }

        return LookupToken(id)->spelledRange;
    }
    auto LanguageQueryProvider::LookupExpandedTextRange(SyntaxTokenID id) const -> TextRange
    {
        return LookupToken(id)->expandedRange;
    }
    auto LanguageQueryProvider::LookupExpandedTextRange(AstSyntaxRange range) const -> TextRange
    {
        if (range.Empty()) {
            auto firstTokenRange = LookupExpandedTextRange(range.GetBeginID());
            return TextRange{firstTokenRange.start, firstTokenRange.start};
        }
        else {
            TextPosition startPosition = LookupExpandedTextRange(range.GetBeginID()).start;
            TextPosition endPosition   = LookupExpandedTextRange(range.GetBackID()).end;
            return TextRange{startPosition, endPosition};
        }
    }
    auto LanguageQueryProvider::LookupExpandedTextRangeExtended(AstSyntaxRange range) const -> TextRange
    {
        if (range.Empty()) {
            auto firstTokenRange = LookupExpandedTextRange(range.GetBeginID());
            return TextRange{firstTokenRange.start, firstTokenRange.start};
        }
        else {
            TextPosition startPosition = LookupExpandedTextRange(range.GetBeginID()).start;
            TextPosition endPosition   = LookupExpandedTextRange(range.GetEndID()).start;
            return TextRange{startPosition, endPosition};
        }
    }
} // namespace glsld