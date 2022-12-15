#pragma once

#include "Compiler.h"
#include "SourceText.h"
#include "SyntaxToken.h"

namespace glsld
{
    enum class DeclTokenType
    {
        Unknown,
        LayoutQualifier,
        Variable,
        Swizzle,
        MemberVariable,
        Parameter,
        Function,
        Type,
        InterfaceBlock,
    };

    struct DeclTokenLookupResult
    {
        SyntaxToken token;
        TextRange range;
        DeclView accessedDecl;
        DeclTokenType accessType;
    };

    static auto FindDeclToken(CompiledModule& compiler, TextPosition position) -> std::optional<DeclTokenLookupResult>
    {
        class DeclTokenVisitor : public ModuleVisitor<DeclTokenVisitor>
        {
        public:
            DeclTokenVisitor(CompiledModule& compiler, TextPosition position)
                : ModuleVisitor<DeclTokenVisitor>(compiler), position(position)
            {
            }

            auto Execute() -> std::optional<DeclTokenLookupResult>
            {
                result = std::nullopt;

                this->Traverse();

                return std::move(result);
            }

            auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
            {
                // we already find the hover
                if (result) {
                    return AstVisitPolicy::Leave;
                }

                TextRange range = this->GetLexContext().LookupTextRange(node.GetRange());
                if (range.Contains(position)) {
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
                    TryDeclToken(type.GetTypeNameTok(), type.GetResolvedStructDecl(), DeclTokenType::Type);
                }
            }

            auto VisitAstTypeQualifierSeq(AstTypeQualifierSeq& quals) -> void
            {
                for (const auto& layoutQual : quals.GetLayoutQuals()) {
                    TryDeclToken(layoutQual.idToken, DeclView{}, DeclTokenType ::LayoutQualifier);
                }
            }

            auto VisitAstStructDecl(AstStructDecl& decl) -> void
            {
                if (decl.GetDeclToken()) {
                    TryDeclToken(*decl.GetDeclToken(), &decl, DeclTokenType::Type);
                }
            }

            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                TryDeclToken(decl.GetName(), &decl, DeclTokenType::Function);
            }

            auto VisitAstParamDecl(AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator()) {
                    TryDeclToken(decl.GetDeclarator()->declTok, &decl, DeclTokenType::Parameter);
                }
            }

            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryDeclToken(declarator.declTok, DeclView{&decl, declaratorIndex}, DeclTokenType::Variable);

                    declaratorIndex += 1;
                }
            }

            auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    TryDeclToken(declarator.declTok, DeclView{&decl, declaratorIndex}, DeclTokenType::MemberVariable);

                    declaratorIndex += 1;
                }
            }

            auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
            {
                if (decl.GetDeclarator()) {
                    TryDeclToken(decl.GetDeclarator()->declTok, &decl, DeclTokenType::InterfaceBlock);
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
            {
                DeclTokenType accessType;
                switch (expr.GetAccessType()) {
                case NameAccessType::Unknown:
                    accessType = DeclTokenType::Unknown;
                    break;
                case NameAccessType::Variable:
                    accessType = DeclTokenType::Variable;
                    break;
                case NameAccessType::Function:
                    accessType = DeclTokenType::Function;
                    break;
                case NameAccessType::Constructor:
                    accessType = DeclTokenType::Type;
                    break;
                case NameAccessType::Swizzle:
                    accessType = DeclTokenType::Swizzle;
                    break;
                }

                TryDeclToken(expr.GetAccessName(), expr.GetAccessedDecl(), accessType);
            }

            auto TryDeclToken(const SyntaxToken& token, DeclView declView, DeclTokenType type) -> void
            {
                if (token.IsIdentifier()) {
                    auto tokRange = this->GetLexContext().LookupTextRange(token);
                    if (tokRange.Contains(position)) {
                        GLSLD_ASSERT(!result);
                        result = DeclTokenLookupResult{
                            .token        = token,
                            .range        = tokRange,
                            .accessedDecl = declView,
                            .accessType   = type,
                        };
                    }
                }
            }

            std::optional<DeclTokenLookupResult> result = std::nullopt;

            TextPosition position;
        };

        return DeclTokenVisitor{compiler, position}.Execute();
    }
} // namespace glsld