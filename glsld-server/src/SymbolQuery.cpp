#include "SymbolQuery.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    namespace
    {
        class SymbolQueryVisitor : public LanguageQueryVisitor<SymbolQueryVisitor>
        {
        private:
            std::optional<SymbolQueryResult> result = std::nullopt;

            TextPosition cursorPos;

            auto TryAstToken(const AstSyntaxToken& token, const AstNode* owner, DeclView declView, SymbolDeclType type,
                             bool isDeclaration) -> void
            {
                if (token.IsIdentifier()) {
                    if (GetProvider().ContainsPosition(token, cursorPos)) {
                        GLSLD_ASSERT(!result);
                        result = SymbolQueryResult{
                            .symbolType          = type,
                            .symbolRange         = token.GetSyntaxRange(),
                            .spelledText         = token.text.Str(),
                            .spelledRange        = GetProvider().LookupExpandedTextRange(token),
                            .astSymbolOccurrence = owner,
                            .symbolDecl          = declView,
                            .isDeclaration       = isDeclaration,
                        };
                    }
                }
            }

        public:
            SymbolQueryVisitor(const LanguageQueryProvider& provider, TextPosition cursorPos)
                : LanguageQueryVisitor(provider), cursorPos(cursorPos)
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
                    TryAstToken(layoutQual.idToken, &quals, DeclView{}, SymbolDeclType::LayoutQualifier, false);
                }
            }

            auto VisitAstQualType(const AstQualType& type) -> void
            {
                if (!type.GetStructDecl()) {
                    // NOTE we handle struct decl at `VisitAstStructDecl`
                    auto resolvedType = type.GetResolvedType();
                    if (auto structDesc = type.GetResolvedType()->GetStructDesc()) {
                        TryAstToken(type.GetTypeNameTok(), &type, structDesc->decl, SymbolDeclType::Type, false);
                    }
                    else if (type.GetResolvedType()->IsBuiltin()) {
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
                if (expr.GetResolvedDecl().IsValid()) {
                    auto decl = expr.GetResolvedDecl().GetDecl();
                    if (decl->Is<AstParamDecl>()) {
                        accessType = SymbolDeclType::Parameter;
                    }
                    else if (decl->Is<AstInterfaceBlockDecl>()) {
                        accessType = SymbolDeclType::BlockInstance;
                    }
                    else if (decl->Is<AstBlockFieldDecl>()) {
                        accessType = SymbolDeclType::BlockMember;
                    }
                    else if (auto varDecl = decl->As<AstVariableDecl>()) {
                        if (varDecl->GetScope() != DeclScope::Global) {
                            accessType = SymbolDeclType::LocalVariable;
                        }
                    }
                }
                TryAstToken(expr.GetNameToken(), &expr, expr.GetResolvedDecl(), accessType, false);
            }
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
            {
                auto declType =
                    (expr.GetResolvedDecl().IsValid() && expr.GetResolvedDecl().GetDecl()->Is<AstBlockFieldDecl>())
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

            auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryAstToken(declarator.nameToken, &decl, DeclView{&decl, declaratorIndex},
                                decl.GetScope() == DeclScope::Global ? SymbolDeclType::GlobalVariable
                                                                     : SymbolDeclType::LocalVariable,
                                true);

                    declaratorIndex += 1;
                }
            }
            auto VisitAstStructFieldDecl(const AstStructFieldDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryAstToken(declarator.nameToken, &decl, DeclView{&decl, declaratorIndex},
                                SymbolDeclType::StructMember, true);

                    declaratorIndex += 1;
                }
            }
            auto VisitAstStructDecl(const AstStructDecl& decl) -> void
            {
                if (decl.GetNameToken()) {
                    TryAstToken(*decl.GetNameToken(), &decl, &decl, SymbolDeclType::Type, true);
                }
            }
            auto VisitAstBlockFieldDecl(const AstBlockFieldDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryAstToken(declarator.nameToken, &decl, DeclView{&decl, declaratorIndex},
                                SymbolDeclType::BlockMember, true);

                    declaratorIndex += 1;
                }
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

    auto QuerySymbolByPosition(const LanguageQueryProvider& provider, TextPosition position)
        -> std::optional<SymbolQueryResult>
    {
        // First, we need to do a binary search to see if the cursor is on a macro/header name.
        // These are not in the AST, so we need to check them first.
        if (auto ppSymbolOccurence = provider.GetPreprocessInfoCache().FindPPSymbolOccurrence(position);
            ppSymbolOccurence) {
            SymbolDeclType symbolType;
            std::string spelledText;
            if (auto headerNameInfo = ppSymbolOccurence->GetHeaderNameInfo(); headerNameInfo) {
                symbolType  = SymbolDeclType::HeaderName;
                spelledText = headerNameInfo->headerName;
            }
            else if (auto macroDefinitionInfo = ppSymbolOccurence->GetMacroDefinitionInfo(); macroDefinitionInfo) {
                symbolType  = SymbolDeclType::Macro;
                spelledText = macroDefinitionInfo->macroName.text.Str();
            }
            else if (auto macroUsageInfo = ppSymbolOccurence->GetMacroUsageInfo(); macroUsageInfo) {
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
        return SymbolQueryVisitor{provider, position}.Execute();
    }
} // namespace glsld