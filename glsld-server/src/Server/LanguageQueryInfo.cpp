#include "Server/LanguageQueryInfo.h"
#include "Server/LanguageQueryVisitor.h"

namespace glsld
{
    auto LanguageQueryInfo::LookupToken(SyntaxTokenID id) const -> const RawSyntaxToken*
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
    auto LanguageQueryInfo::LookupTokens(AstSyntaxRange range) const -> ArrayView<RawSyntaxToken>
    {
        return {LookupToken(range.GetBeginID()), LookupToken(range.GetEndID())};
    }
    auto LanguageQueryInfo::LookupDotTokenIndex(const AstExpr& expr) const -> std::optional<SyntaxTokenID>
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
    auto LanguageQueryInfo::LookupTokenByPosition(TextPosition position) const -> ArrayView<RawSyntaxToken>
    {
        auto tokens           = compilerResult->GetUserFileArtifacts().GetTokens();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            tokens, position, {}, [](const RawSyntaxToken& tok) { return tok.expandedRange.start; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryInfo::LookupTokenByLine(uint32_t lineNum) const -> ArrayView<RawSyntaxToken>
    {
        auto tokens           = compilerResult->GetUserFileArtifacts().GetTokens();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            tokens, lineNum, {}, [](const RawSyntaxToken& tok) { return tok.expandedRange.start.line; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryInfo::LookupPreceedingComment(SyntaxTokenID id) const -> ArrayView<RawCommentToken>
    {
        if (id.GetTU() != TranslationUnitID::UserFile) {
            return {};
        }

        auto comments         = compilerResult->GetUserFileArtifacts().GetComments();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            comments, id.GetTokenIndex(), {}, [](const RawCommentToken& token) { return token.nextTokenIndex; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryInfo::LookupSpelledFile(SyntaxTokenID id) const -> FileID
    {
        return LookupToken(id)->spelledFile;
    }
    auto LanguageQueryInfo::IsMainFile(FileID file) const -> bool
    {
        return file == compilerResult->GetUserFileArtifacts().GetTokens().back().spelledFile;
    }
    auto LanguageQueryInfo::IsSpelledInMainFile(SyntaxTokenID id) const -> bool
    {
        if (id.GetTU() != TranslationUnitID::UserFile) {
            return false;
        }

        return LookupSpelledFile(id) == compilerResult->GetUserFileArtifacts().GetTokens().back().spelledFile;
    }
    auto LanguageQueryInfo::LookupSpelledTextRange(SyntaxTokenID id) const -> FileTextRange
    {
        auto token = LookupToken(id);
        return FileTextRange{
            .fileID = token->spelledFile,
            .range  = token->spelledRange,
        };
    }
    auto LanguageQueryInfo::LookupSpelledTextRangeInMainFile(SyntaxTokenID id) const -> std::optional<TextRange>
    {
        if (!IsSpelledInMainFile(id)) {
            return std::nullopt;
        }

        return LookupToken(id)->spelledRange;
    }
    auto LanguageQueryInfo::LookupExpandedTextRange(SyntaxTokenID id) const -> TextRange
    {
        return LookupToken(id)->expandedRange;
    }
    auto LanguageQueryInfo::LookupExpandedTextRange(AstSyntaxRange range) const -> TextRange
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
    auto LanguageQueryInfo::LookupExpandedTextRangeExtended(AstSyntaxRange range) const -> TextRange
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

    namespace
    {
        class SymbolQueryVisitor : public LanguageQueryVisitor<SymbolQueryVisitor>
        {
        private:
            std::optional<SymbolQueryResult> result = std::nullopt;

            TextPosition cursorPos;

            auto TryAstToken(const AstSyntaxToken& token, const AstNode* owner, const AstDecl* decl,
                             SymbolDeclType type, bool isDeclaration) -> void
            {
                if (token.IsIdentifier()) {
                    if (GetInfo().ContainsPosition(token, cursorPos)) {
                        GLSLD_ASSERT(!result);
                        result = SymbolQueryResult{
                            .symbolType          = type,
                            .symbolRange         = token.GetSyntaxRange(),
                            .spelledText         = token.text.Str(),
                            .spelledRange        = GetInfo().LookupExpandedTextRange(token),
                            .astSymbolOccurrence = owner,
                            .symbolDecl          = decl,
                            .isDeclaration       = isDeclaration,
                        };
                    }
                }
            }

        public:
            SymbolQueryVisitor(const LanguageQueryInfo& info, TextPosition cursorPos)
                : LanguageQueryVisitor(info), cursorPos(cursorPos)
            {
            }

            auto Execute() -> std::optional<SymbolQueryResult>
            {
                result = std::nullopt;

                TraverseTranslationUnit();
                return std::move(result);
            }

            auto EnterAstNode(const AstNode& node) -> AstVisitPolicy
            {
                if (result.has_value()) {
                    return AstVisitPolicy::Halt;
                }

                if (GetInfo().ContainsPosition(node, cursorPos)) {
                    return AstVisitPolicy::Traverse;
                }
                else if (GetInfo().PrecedesPosition(node, cursorPos)) {
                    return AstVisitPolicy::Leave;
                }
                else {
                    return AstVisitPolicy::Halt;
                }
            }

            auto VisitAstTypeQualifierSeq(const AstTypeQualifierSeq& quals) -> void
            {
                for (const auto& layoutQual : quals.GetLayoutQuals()) {
                    TryAstToken(layoutQual.idToken, &quals, nullptr, SymbolDeclType::LayoutQualifier, false);
                }
            }

            auto VisitAstQualType(const AstQualType& type) -> void
            {
                // NOTE we handle struct decl at `VisitAstStructDecl`
                if (!type.GetStructDecl()) {

                    // We need to unwrap the arrays to get the base type identifier
                    auto innerType = type.GetResolvedType();
                    while (auto arrayDesc = innerType->GetArrayDesc()) {
                        innerType = arrayDesc->elementType;
                    }

                    if (auto structDesc = innerType->GetStructDesc()) {
                        // Struct Name
                        TryAstToken(type.GetTypeNameTok(), &type, structDesc->decl, SymbolDeclType::Type, false);
                    }
                    else if (innerType->IsBuiltin()) {
                        // Builtin type
                        TryAstToken(type.GetTypeNameTok(), &type, nullptr, SymbolDeclType::Type, false);
                    }
                    else if (type.GetResolvedType()->IsError()) {
                        // Unknown type
                        TryAstToken(type.GetTypeNameTok(), &type, nullptr, SymbolDeclType::Type, false);
                    }

                    // FIXME: any other case?
                }
            }

            auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void
            {
                // By default, we assume it's a global variable access.
                SymbolDeclType accessType = SymbolDeclType::GlobalVariable;
                if (expr.GetResolvedDecl()) {
                    auto decl = expr.GetResolvedDecl();
                    if (decl->Is<AstParamDecl>()) {
                        accessType = SymbolDeclType::Parameter;
                    }
                    else if (decl->Is<AstInterfaceBlockDecl>()) {
                        accessType = SymbolDeclType::BlockInstance;
                    }
                    else if (decl->Is<AstBlockFieldDeclaratorDecl>()) {
                        accessType = SymbolDeclType::BlockMember;
                    }
                    else if (auto varDecl = decl->As<AstVariableDeclaratorDecl>()) {
                        if (varDecl->GetScope() != DeclScope::Global) {
                            accessType = SymbolDeclType::LocalVariable;
                        }
                    }
                }
                TryAstToken(expr.GetNameToken(), &expr, expr.GetResolvedDecl(), accessType, false);
            }
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
            {
                auto resolvedDecl = expr.GetResolvedDecl();
                auto declType     = (resolvedDecl && resolvedDecl->Is<AstBlockFieldDeclaratorDecl>())
                                        ? SymbolDeclType::BlockMember
                                        : SymbolDeclType::StructMember;

                TryAstToken(expr.GetNameToken(), &expr, expr.GetResolvedDecl(), declType, false);
            }
            auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void
            {
                TryAstToken(expr.GetNameToken(), &expr, {}, SymbolDeclType::Swizzle, false);
            }
            auto VisitAstUnaryExpr(const AstUnaryExpr& expr) -> void
            {
                // FIXME: .length()
            }
            auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
            {
                TryAstToken(expr.GetNameToken(), &expr, expr.GetResolvedFunction(), SymbolDeclType::Function, false);
            }

            auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl) -> void
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl,
                            decl.GetScope() == DeclScope::Global ? SymbolDeclType::GlobalVariable
                                                                 : SymbolDeclType::LocalVariable,
                            true);
            }
            auto VisitAstStructFieldDeclaratorDecl(const AstStructFieldDeclaratorDecl& decl) -> void
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::StructMember, true);
            }
            auto VisitAstStructDecl(const AstStructDecl& decl) -> void
            {
                if (decl.GetNameToken()) {
                    TryAstToken(*decl.GetNameToken(), &decl, &decl, SymbolDeclType::Type, true);
                }
            }
            auto VisitAstBlockFieldDeclaratorDecl(const AstBlockFieldDeclaratorDecl& decl) -> void
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::BlockMember, true);
            }
            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
            {
                // FIXME: explain the symbol access type
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::Block, true);
                if (decl.GetDeclarator()) {
                    TryAstToken(decl.GetDeclarator()->nameToken, &decl, &decl, SymbolDeclType::BlockInstance, true);
                }
            }
            auto VisitAstParamDecl(const AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator()) {
                    TryAstToken(decl.GetDeclarator()->nameToken, &decl, &decl, SymbolDeclType::Parameter, true);
                }
            }
            auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::Function, true);
            }
        };
    } // namespace

    auto LanguageQueryInfo::QuerySymbolByPosition(TextPosition position) const -> std::optional<SymbolQueryResult>
    {
        // First, we need to do a binary search to see if the cursor is on a macro/header name.
        // These are not in the AST, so we need to check them first.
        if (auto ppSymbolOccurence = GetPreprocessInfo().QueryPPSymbol(position); ppSymbolOccurence) {
            SymbolDeclType symbolType;
            std::string spelledText;
            if (auto headerNameInfo = ppSymbolOccurence->GetHeaderNameInfo(); headerNameInfo) {
                symbolType  = SymbolDeclType::HeaderName;
                spelledText = headerNameInfo->headerName.text.Str();
            }
            else if (auto macroUsageInfo = ppSymbolOccurence->GetMacroInfo(); macroUsageInfo) {
                symbolType  = SymbolDeclType::Macro;
                spelledText = macroUsageInfo->macroName.text.Str();
            }

            return SymbolQueryResult{
                .symbolType         = symbolType,
                .symbolRange        = {},
                .spelledText        = std::move(spelledText),
                .spelledRange       = ppSymbolOccurence->GetSpelledRange(),
                .ppSymbolOccurrence = ppSymbolOccurence,
            };
        }

        // Then, we traverse the AST to search for the identifier that the cursor is on, if any.
        return SymbolQueryVisitor{*this, position}.Execute();
    }
} // namespace glsld