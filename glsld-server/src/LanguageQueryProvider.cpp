#include "LanguageQueryProvider.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    auto LanguageQueryProvider::LookupSymbolAccess(TextPosition position) const -> std::optional<SymbolAccessInfo>
    {
        class DeclTokenVisitor : public LanguageQueryVisitor<DeclTokenVisitor>
        {
        private:
            std::optional<SymbolAccessInfo> result = std::nullopt;

            TextPosition cursorPos;

        public:
            DeclTokenVisitor(const LanguageQueryProvider& provider, TextPosition cursorPos)
                : LanguageQueryVisitor(provider), cursorPos(cursorPos)
            {
            }

            auto Execute() -> std::optional<SymbolAccessInfo>
            {
                result = std::nullopt;

                TraverseTranslationUnit();
                return std::move(result);
            }

            auto EnterAstNode(const AstNode& node) -> AstVisitPolicy
            {
                if (GetProvider().ContainsPosition(node, cursorPos)) {
                    return AstVisitPolicy::Traverse;
                }
                else if (GetProvider().PrecedesPosition(node, cursorPos)) {
                    return AstVisitPolicy::Leave;
                }
                else {
                    return AstVisitPolicy::Halt;
                }
            }

            auto VisitAstTypeQualifierSeq(const AstTypeQualifierSeq& quals) -> void
            {
                for (const auto& layoutQual : quals.GetLayoutQuals()) {
                    TryDeclToken(layoutQual.idToken, DeclView{}, SymbolAccessType::LayoutQualifier, false);
                }
            }

            auto VisitAstQualType(const AstQualType& type) -> void
            {
                if (!type.GetStructDecl()) {
                    // NOTE we handle struct decl at `VisitAstStructDecl`
                    auto resolvedType = type.GetResolvedType();
                    if (auto structDesc = type.GetResolvedType()->GetStructDesc()) {
                        TryDeclToken(type.GetTypeNameTok(), structDesc->decl, SymbolAccessType::Type, false);
                    }
                    else if (type.GetResolvedType()->IsBuiltin()) {
                        TryDeclToken(type.GetTypeNameTok(), nullptr, SymbolAccessType::Type, false);
                    }
                    else if (type.GetResolvedType()->IsError()) {
                        // Unknown type
                        TryDeclToken(type.GetTypeNameTok(), nullptr, SymbolAccessType::Type, false);
                    }

                    // FIXME: any other case?
                }
            }

            auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void
            {
                // By default, we assume it's a global variable access.
                SymbolAccessType accessType = SymbolAccessType::GlobalVariable;
                if (expr.GetResolvedDecl().IsValid()) {
                    auto decl = expr.GetResolvedDecl().GetDecl();
                    if (decl->Is<AstParamDecl>()) {
                        accessType = SymbolAccessType::Parameter;
                    }
                    else if (decl->Is<AstInterfaceBlockDecl>()) {
                        accessType = SymbolAccessType::InterfaceBlockInstance;
                    }
                    else if (auto varDecl = decl->As<AstVariableDecl>()) {
                        if (varDecl->GetScope() != DeclScope::Global) {
                            accessType = SymbolAccessType::LocalVariable;
                        }
                    }
                }
                TryDeclToken(expr.GetAccessName(), expr.GetResolvedDecl(), accessType, false);
            }
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
            {
                TryDeclToken(expr.GetAccessName(), expr.GetResolvedDecl(), SymbolAccessType::MemberVariable, false);
            }
            auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void
            {
                TryDeclToken(expr.GetAccessName(), {}, SymbolAccessType::Swizzle, false);
            }
            auto VisitAstUnaryExpr(const AstUnaryExpr& expr) -> void
            {
                // FIXME: .length()
            }
            auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
            {
                TryDeclToken(expr.GetFunctionName(), expr.GetResolvedFunction(), SymbolAccessType::Function, false);
            }

            auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryDeclToken(declarator.declTok, DeclView{&decl, declaratorIndex},
                                 decl.GetScope() == DeclScope::Global ? SymbolAccessType::GlobalVariable
                                                                      : SymbolAccessType::LocalVariable,
                                 true);

                    declaratorIndex += 1;
                }
            }
            auto VisitAstFieldDecl(const AstFieldDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryDeclToken(declarator.declTok, DeclView{&decl, declaratorIndex}, SymbolAccessType::MemberVariable,
                                 true);

                    declaratorIndex += 1;
                }
            }
            auto VisitAstStructDecl(const AstStructDecl& decl) -> void
            {
                if (decl.GetDeclTok()) {
                    TryDeclToken(*decl.GetDeclTok(), &decl, SymbolAccessType::Type, true);
                }
            }
            auto VisitAstParamDecl(const AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator()) {
                    TryDeclToken(decl.GetDeclarator()->declTok, &decl, SymbolAccessType::Parameter, true);
                }
            }
            auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
            {
                TryDeclToken(decl.GetDeclTok(), &decl, SymbolAccessType::Function, true);
            }
            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
            {
                // FIXME: explain the symbol access type
                TryDeclToken(decl.GetDeclTok(), &decl, SymbolAccessType::InterfaceBlock, true);
                if (decl.GetDeclarator()) {
                    TryDeclToken(decl.GetDeclarator()->declTok, &decl, SymbolAccessType::InterfaceBlockInstance, true);
                }
            }

            auto TryDeclToken(const SyntaxToken& token, DeclView declView, SymbolAccessType type, bool isDeclName)
                -> void
            {
                if (token.IsIdentifier()) {
                    if (GetProvider().ContainsPosition(token, cursorPos)) {
                        GLSLD_ASSERT(!result);
                        result = SymbolAccessInfo{
                            .token      = token,
                            .symbolDecl = declView,
                            .symbolType = type,
                            .isDeclName = isDeclName,
                        };
                    }
                }
            }
        };

        return DeclTokenVisitor{*this, position}.Execute();
    }

    auto LanguageQueryProvider::LookupToken(SyntaxTokenID id) const -> const RawSyntaxTokenEntry*
    {
        ArrayView<RawSyntaxTokenEntry> tokens;
        switch (id.GetTU()) {
        case TranslationUnitID::SystemPreamble:
            tokens = compilerResult->GetSystemPreambleTokens();
            break;
        case TranslationUnitID::UserPreamble:
            tokens = compilerResult->GetUserPreambleTokens();
            break;
        case TranslationUnitID::UserFile:
            tokens = compilerResult->GetUserFileTokens();
            break;
        }

        return &tokens[id.GetTokenIndex()];
    }
    auto LanguageQueryProvider::LookupTokens(AstSyntaxRange range) const -> ArrayView<RawSyntaxTokenEntry>
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
    auto LanguageQueryProvider::LookupTokenByPosition(TextPosition position) const -> ArrayView<RawSyntaxTokenEntry>
    {
        auto tokens           = compilerResult->GetUserFileTokens();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            tokens, position, {}, [](const RawSyntaxTokenEntry& tok) { return tok.expandedRange.start; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryProvider::LookupTokenByLine(uint32_t lineNum) const -> ArrayView<RawSyntaxTokenEntry>
    {
        auto tokens           = compilerResult->GetUserFileTokens();
        auto [itBegin, itEnd] = std::ranges::equal_range(
            tokens, lineNum, {}, [](const RawSyntaxTokenEntry& tok) { return tok.expandedRange.start.line; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryProvider::LookupPreceedingComment(SyntaxTokenID id) const -> ArrayView<RawCommentTokenEntry>
    {
        if (id.GetTU() != TranslationUnitID::UserFile) {
            return {};
        }

        auto allComments = compilerResult->GetUserFileComments();
        auto [itBegin, itEnd] =
            std::ranges::equal_range(allComments, id.GetTokenIndex(), {},
                                     [](const RawCommentTokenEntry& entry) { return entry.nextTokenIndex; });
        return {std::to_address(itBegin), std::to_address(itEnd)};
    }
    auto LanguageQueryProvider::LookupSpelledFile(SyntaxTokenID id) const -> FileID
    {
        return LookupToken(id)->spelledFile;
    }
    auto LanguageQueryProvider::IsSpelledInMainFile(SyntaxTokenID id) const -> bool
    {
        if (id.GetTU() != TranslationUnitID::UserFile) {
            return false;
        }

        return LookupSpelledFile(id) == compilerResult->GetMainFileID();
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