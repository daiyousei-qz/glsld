#pragma once

#include "Compiler.h"
#include "SourceText.h"
#include "SyntaxToken.h"

namespace glsld
{
    // template <typename T>
    // struct DeclTokenCallback2
    // {
    //     virtual auto ProcessToken(std::string_view id, TextRange range, AstDecl& decl) const -> std::optional<T> = 0;
    // };

    template <typename T>
    struct DeclTokenCallback
    {
    public:
        virtual auto ProcessToken(SyntaxToken token, TextRange range, AstDecl& decl) const -> std::optional<T> = 0;
        virtual auto ProcessTokenWithoutDecl(SyntaxToken token, TextRange range, NameAccessType type) const
            -> std::optional<T> = 0;
    };

    struct DeclTokenLookupResult
    {
        SyntaxToken token;
        TextRange range;
        DeclView accessedDecl;
        NameAccessType accessType;
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
                    TextRange range = this->GetLexContext().LookupTextRange(type.GetTypeNameTok());
                    if (range.Contains(position)) {
                        // FIXME: implement this
                    }
                }
            }

            auto VisitAstStructDecl(AstStructDecl& decl) -> void
            {
                if (decl.GetDeclToken()) {
                    auto declToken = *decl.GetDeclToken();
                    auto declRange = this->GetLexContext().LookupTextRange(declToken);
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!result);
                        result = DeclTokenLookupResult{
                            .token        = declToken,
                            .range        = declRange,
                            .accessedDecl = &decl,
                            // FIXME: access type unknown?
                            .accessType = NameAccessType::Unknown,
                        };
                    }
                }
            }

            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                auto declRange = this->GetLexContext().LookupTextRange(decl.GetName());
                if (declRange.Contains(position)) {
                    GLSLD_ASSERT(!result);
                    result = DeclTokenLookupResult{
                        .token        = decl.GetName(),
                        .range        = declRange,
                        .accessedDecl = &decl,
                        .accessType   = NameAccessType::Function,
                    };
                }
            }

            auto VisitAstParamDecl(AstParamDecl& decl) -> void
            {
                if (decl.GetDeclToken()) {
                    auto declToken = *decl.GetDeclToken();
                    auto declRange = this->GetLexContext().LookupTextRange(*decl.GetDeclToken());
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!result);
                        result = DeclTokenLookupResult{
                            .token        = declToken,
                            .range        = declRange,
                            .accessedDecl = &decl,
                            .accessType   = NameAccessType::Variable,
                        };
                    }
                }
            }

            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    auto declRange = this->GetLexContext().LookupTextRange(declarator.declTok);
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!result);
                        result = DeclTokenLookupResult{
                            .token        = declarator.declTok,
                            .range        = declRange,
                            .accessedDecl = DeclView{&decl, declaratorIndex},
                            .accessType   = NameAccessType::Variable,
                        };
                    }

                    declaratorIndex += 1;
                }
            }

            auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
            {
                size_t declaratorIndex = 0;
                for (const auto& declarator : decl.GetDeclarators()) {
                    auto declRange = this->GetLexContext().LookupTextRange(declarator.declTok);
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!result);
                        result = DeclTokenLookupResult{
                            .token        = declarator.declTok,
                            .range        = declRange,
                            .accessedDecl = DeclView{&decl, declaratorIndex},
                            .accessType   = NameAccessType::Variable,
                        };
                    }

                    declaratorIndex += 1;
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
            {
                auto declToken = expr.GetAccessName();
                auto declRange = this->GetLexContext().LookupTextRange(declToken);
                if (declRange.Contains(position)) {
                    GLSLD_ASSERT(!result);
                    result = DeclTokenLookupResult{
                        .token        = declToken,
                        .range        = declRange,
                        .accessedDecl = expr.GetAccessedDecl(),
                        .accessType   = expr.GetAccessType(),
                    };
                }
            }

            std::optional<DeclTokenLookupResult> result = std::nullopt;

            TextPosition position;
        };

        return DeclTokenVisitor{compiler, position}.Execute();
    }
} // namespace glsld