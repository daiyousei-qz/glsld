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
} // namespace glsld