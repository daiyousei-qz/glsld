#include "Server/LanguageQueryInfo.h"
#include "Compiler/CompilerArtifacts.h"
#include "Compiler/SyntaxToken.h"
#include "Server/LanguageQueryVisitor.h"

namespace glsld
{
    auto LanguageQueryInfo::LookupToken(SyntaxTokenID id) const -> const RawSyntaxToken*
    {
        return &LookupArtifact(id.GetTU())->GetTokens()[id.GetTokenIndex()];
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
        auto comments         = LookupArtifact(id.GetTU())->GetComments();
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
        class SymbolQueryVisitor : public LanguageQueryVisitor<SymbolQueryVisitor, std::optional<SymbolQueryResult>>
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

            auto Finish() -> std::optional<SymbolQueryResult> GLSLD_AST_VISITOR_OVERRIDE
            {
                return std::move(result);
            }

            auto EnterAstNode(const AstNode& node) -> AstVisitPolicy GLSLD_AST_VISITOR_OVERRIDE
            {
                if (result.has_value()) {
                    return AstVisitPolicy::Halt;
                }

                return TraverseNodeContains(node, cursorPos);
            }

            auto VisitAstTypeQualifierSeq(const AstTypeQualifierSeq& quals) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                for (const auto& layoutQual : quals.GetLayoutQuals()) {
                    TryAstToken(layoutQual.idToken, &quals, nullptr, SymbolDeclType::LayoutQualifier, false);
                }
            }

            auto VisitAstQualType(const AstQualType& type) -> void GLSLD_AST_VISITOR_OVERRIDE
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

            auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
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

            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                auto resolvedDecl = expr.GetResolvedDecl();
                auto declType     = (resolvedDecl && resolvedDecl->Is<AstBlockFieldDeclaratorDecl>())
                                        ? SymbolDeclType::BlockMember
                                        : SymbolDeclType::StructMember;

                TryAstToken(expr.GetNameToken(), &expr, expr.GetResolvedDecl(), declType, false);
            }

            auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                TryAstToken(expr.GetNameToken(), &expr, {}, SymbolDeclType::Swizzle, false);
            }

            auto VisitAstUnaryExpr(const AstUnaryExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                // FIXME: .length()
            }

            auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                TryAstToken(expr.GetNameToken(), &expr, expr.GetResolvedFunction(), SymbolDeclType::Function, false);
            }

            auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl)
                -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl,
                            decl.GetScope() == DeclScope::Global ? SymbolDeclType::GlobalVariable
                                                                 : SymbolDeclType::LocalVariable,
                            true);
            }

            auto VisitAstStructFieldDeclaratorDecl(const AstStructFieldDeclaratorDecl& decl)
                -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::StructMember, true);
            }

            auto VisitAstStructDecl(const AstStructDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (decl.GetNameToken()) {
                    TryAstToken(*decl.GetNameToken(), &decl, &decl, SymbolDeclType::Type, true);
                }
            }
            auto VisitAstBlockFieldDeclaratorDecl(const AstBlockFieldDeclaratorDecl& decl)
                -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::BlockMember, true);
            }

            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                // FIXME: explain the symbol access type
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::Block, true);
                if (decl.GetDeclarator()) {
                    TryAstToken(decl.GetDeclarator()->nameToken, &decl, &decl, SymbolDeclType::BlockInstance, true);
                }
            }

            auto VisitAstParamDecl(const AstParamDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (decl.GetDeclarator()) {
                    TryAstToken(decl.GetDeclarator()->nameToken, &decl, &decl, SymbolDeclType::Parameter, true);
                }
            }

            auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                TryAstToken(decl.GetNameToken(), &decl, &decl, SymbolDeclType::Function, true);
            }
        };
    } // namespace

    auto LanguageQueryInfo::QuerySymbolByPosition(TextPosition position) const -> std::optional<SymbolQueryResult>
    {
        // First, we need to do a binary search to see if the cursor is on a macro/header name.
        // These are not in the AST, so we need to check them first.
        if (auto ppSymbolOccurrence = GetPreprocessInfo().QueryPPSymbol(position); ppSymbolOccurrence) {
            SymbolDeclType symbolType;
            std::string spelledText;
            bool isDeclaration = false;
            if (auto headerNameInfo = ppSymbolOccurrence->GetHeaderNameInfo(); headerNameInfo) {
                symbolType  = SymbolDeclType::HeaderName;
                spelledText = headerNameInfo->headerName.text.Str();
            }
            else if (auto macroUsageInfo = ppSymbolOccurrence->GetMacroInfo(); macroUsageInfo) {
                symbolType    = SymbolDeclType::Macro;
                spelledText   = macroUsageInfo->macroName.text.Str();
                isDeclaration = macroUsageInfo->occurrenceType == PPMacroOccurrenceType::Define;
            }

            return SymbolQueryResult{
                .symbolType         = symbolType,
                .symbolRange        = {},
                .spelledText        = std::move(spelledText),
                .spelledRange       = ppSymbolOccurrence->GetSpelledRange(),
                .ppSymbolOccurrence = ppSymbolOccurrence,
                .isDeclaration      = isDeclaration,
            };
        }

        // Then, we traverse the AST to search for the identifier that the cursor is on, if any.
        return TraverseAst(SymbolQueryVisitor{*this, position}, GetUserFileAst());
    }

    auto LanguageQueryInfo::QueryCommentDescription(const AstDecl& decl) const -> std::string
    {
        auto declRange = LookupExpandedTextRange(decl.GetSyntaxRange());

        auto unwrapComment = [](const RawCommentToken& token) -> StringView {
            auto commentText = token.text.StrView();
            if (commentText.StartWith("//")) {
                return commentText.Drop(2).Trim();
            }
            else if (commentText.StartWith("/*")) {
                GLSLD_ASSERT(commentText.EndWith("*/"));
                return commentText.Drop(2).DropBack(2);
            }
            return commentText;
        };

        // Case 1: Trailing comment in the same line
        // e.g. `int a; // comment`
        auto trailingComments = LookupPreceedingComment(decl.GetSyntaxRange().GetEndID());
        if (declRange.GetNumLines() == 1 && !trailingComments.empty()) {
            std::string result;
            for (const auto& token : trailingComments) {
                // For a comment to qualify, it must be consecutive comments who:
                // - appears after the declaration
                // - has no non-comment token after it in the same line
                if (token.spelledRange.start.line == declRange.end.line &&
                    token.spelledRange.end.line == declRange.end.line &&
                    token.backAttachmentLine != token.spelledRange.end.line) {
                    result += unwrapComment(token);
                    result += "\n";
                }
            }

            if (!result.empty()) {
                return result;
            }
        }

        // Case 2: Preceeding comments in the previous lines
        // e.g. ```
        //      // comment
        //      int a;
        //      ```
        auto preceedingComments = LookupPreceedingComment(decl.GetSyntaxRange().GetBeginID());
        if (!preceedingComments.empty()) {
            // FIXME: avoid using comments if there are preprocessor lines between them and the declaration
            std::string result;
            for (const auto& token : preceedingComments) {
                // For a comment to qualify, it must be consecutive comments who:
                // - occupies the entire line(s)
                // - back attach to the declaration line (i.e. no preprocessor lines in between)
                if (token.backAttachmentLine == declRange.start.line &&
                    token.frontAttachmentLine != token.spelledRange.start.line) {
                    result += unwrapComment(token);
                    result += "\n";
                }
            }

            return result;
        }

        return "";
    }
} // namespace glsld