#include "LanguageQueryProvider.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    auto LanguageQueryProvider::LookupSymbolAccess(TextPosition position) const -> std::optional<SymbolAccessInfo>
    {
        class DeclTokenVisitor : public LanguageQueryVisitor<DeclTokenVisitor>
        {
        public:
            DeclTokenVisitor(const LanguageQueryProvider& provider, TextPosition cursorPos)
                : LanguageQueryVisitor(provider), cursorPos(cursorPos)
            {
            }

            auto Execute() -> std::optional<SymbolAccessInfo>
            {
                result = std::nullopt;

                TraverseGlobalDeclUntil(cursorPos);
                return std::move(result);
            }

            auto EnterAstNodeBase(AstNode& node) -> AstVisitPolicy
            {
                if (GetProvider().ContainsPosition(node, cursorPos)) {
                    return AstVisitPolicy::Traverse;
                }
                else {
                    return AstVisitPolicy::Leave;
                }
            }

            auto VisitAstQualType(AstQualType& type) -> void
            {
                if (!type.GetStructDecl()) {
                    // NOTE we process struct decl at `VisitAstStructDecl`
                    TryDeclToken(type.GetTypeNameTok(), type.GetResolvedStructDecl(), SymbolAccessType::Type);
                }
            }

            auto VisitAstTypeQualifierSeq(AstTypeQualifierSeq& quals) -> void
            {
                for (const auto& layoutQual : quals.GetLayoutQuals()) {
                    TryDeclToken(layoutQual.idToken, DeclView{}, SymbolAccessType ::LayoutQualifier);
                }
            }

            auto VisitAstStructDecl(AstStructDecl& decl) -> void
            {
                if (decl.GetDeclTok()) {
                    TryDeclToken(*decl.GetDeclTok(), &decl, SymbolAccessType::Type);
                }
            }

            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                TryDeclToken(decl.GetDeclTok(), &decl, SymbolAccessType::Function);
            }

            auto VisitAstParamDecl(AstParamDecl& decl) -> void
            {
                TryDeclToken(decl.GetDeclarator().declTok, &decl, SymbolAccessType::Parameter);
            }

            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryDeclToken(declarator.declTok, DeclView{&decl, declaratorIndex}, SymbolAccessType::Variable);

                    declaratorIndex += 1;
                }
            }

            auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryDeclToken(declarator.declTok, DeclView{&decl, declaratorIndex},
                                 SymbolAccessType::MemberVariable);

                    declaratorIndex += 1;
                }
            }

            auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
            {
                if (decl.GetDeclarator()) {
                    TryDeclToken(decl.GetDeclarator()->declTok, &decl, SymbolAccessType::InterfaceBlockInstance);
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
            {
                SymbolAccessType accessType;
                switch (expr.GetAccessType()) {
                case NameAccessType::Unknown:
                    accessType = SymbolAccessType::Unknown;
                    break;
                case NameAccessType::Variable:
                    accessType = SymbolAccessType::Variable;
                    break;
                case NameAccessType::Function:
                    accessType = SymbolAccessType::Function;
                    break;
                case NameAccessType::Constructor:
                    accessType = SymbolAccessType::Type;
                    break;
                case NameAccessType::Swizzle:
                    accessType = SymbolAccessType::Swizzle;
                    break;
                }

                TryDeclToken(expr.GetAccessName(), expr.GetAccessedDecl(), accessType);
            }

            auto TryDeclToken(const SyntaxToken& token, DeclView declView, SymbolAccessType type) -> void
            {
                if (token.IsIdentifier()) {
                    auto tokRange = GetProvider().GetLexContext().LookupExpandedTextRange(token);
                    if (tokRange.Contains(cursorPos)) {
                        GLSLD_ASSERT(!result);
                        result = SymbolAccessInfo{
                            .token      = token,
                            .symbolDecl = declView,
                            .symbolType = type,
                        };
                    }
                }
            }

            std::optional<SymbolAccessInfo> result = std::nullopt;

            TextPosition cursorPos;
        };

        return DeclTokenVisitor{*this, position}.Execute();
    }
} // namespace glsld