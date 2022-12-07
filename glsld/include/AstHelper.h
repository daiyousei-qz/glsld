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

    template <typename T>
    static auto ProcessDeclToken(CompiledModule& compiler, TextPosition position, const DeclTokenCallback<T>& callback)
        -> std::optional<T>
    {
        class DeclTokenVisitor : public AstVisitor<DeclTokenVisitor>
        {
        public:
            DeclTokenVisitor(CompiledModule& compiler, TextPosition position, const DeclTokenCallback<T>& callback)
                : lexContext(compiler.GetLexContext()), astContext(compiler.GetAstContext()), position(position),
                  callback(callback)
            {
            }

            auto Execute() -> std::optional<T>
            {
                finished = false;
                result   = std::nullopt;

                this->TraverseAst(astContext);

                return std::move(result);
            }

            auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
            {
                // we already find the hover
                if (finished) {
                    return AstVisitPolicy::Leave;
                }

                auto range = lexContext.LookupTextRange(node.GetRange());
                if (range.Contains(position)) {
                    return AstVisitPolicy::Traverse;
                }
                else {
                    return AstVisitPolicy::Leave;
                }
            }

            // auto VisitAstQualType(AstQualType& type) -> void
            // {
            //     auto locBegin = lexContext.LookupSyntaxLocation(type.GetTypeNameTok().range.begin);
            //     auto locEnd   = lexContext.LookupSyntaxLocation(type.GetTypeNameTok().range.end);
            //     if (locBegin.line != position.line || locEnd.line != position.line) {
            //         return;
            //     }
            //     if (locBegin.column <= position.character && locEnd.column >= position.character) {
            //         GLSLD_ASSERT(!finished);
            //         finished = true;
            //         result   = callback.ProcessToken(type.GetTypeNameTok(), GetTextRange(locBegin, locEnd), ???);
            //     }
            // }

            auto VisitAstStructDecl(AstStructDecl& decl) -> void
            {
                if (decl.GetDeclToken()) {
                    auto declRange = lexContext.LookupTextRange(*decl.GetDeclToken());
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!finished);
                        finished = true;
                        result   = callback.ProcessToken(*decl.GetDeclToken(), declRange, decl);
                    }
                }
            }

            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                auto declRange = lexContext.LookupTextRange(decl.GetName());
                if (declRange.Contains(position)) {
                    GLSLD_ASSERT(!finished);
                    finished = true;
                    result   = callback.ProcessToken(decl.GetName(), declRange, decl);
                }
            }

            auto VisitAstParamDecl(AstParamDecl& decl) -> void
            {
                if (decl.GetDeclToken()) {
                    auto declRange = lexContext.LookupTextRange(*decl.GetDeclToken());
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!finished);
                        finished = true;
                        result   = callback.ProcessToken(*decl.GetDeclToken(), declRange, decl);
                    }
                }
            }

            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                for (const auto& declarator : decl.GetDeclarators()) {
                    auto declRange = lexContext.LookupTextRange(declarator.declTok);
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!finished);
                        finished = true;
                        result   = callback.ProcessToken(declarator.declTok, declRange, decl);
                    }
                }
            }

            auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
            {
                for (const auto& declarator : decl.GetDeclarators()) {
                    auto declRange = lexContext.LookupTextRange(declarator.declTok);
                    if (declRange.Contains(position)) {
                        GLSLD_ASSERT(!finished);
                        finished = true;
                        result   = callback.ProcessToken(declarator.declTok, declRange, decl);
                    }
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
            {
                auto declRange = lexContext.LookupTextRange(expr.GetAccessName());
                if (declRange.Contains(position)) {
                    GLSLD_ASSERT(!finished);
                    auto accessedDecl = expr.GetAccessedDecl();
                    finished          = true;
                    if (accessedDecl) {
                        result = callback.ProcessToken(expr.GetAccessName(), declRange, *accessedDecl.GetDecl());
                    }
                    else {
                        result =
                            callback.ProcessTokenWithoutDecl(expr.GetAccessName(), declRange, expr.GetAccessType());
                    }
                }
            }

            bool finished           = false;
            std::optional<T> result = std::nullopt;

            const LexContext& lexContext;
            const AstContext& astContext;
            TextPosition position;
            const DeclTokenCallback<T>& callback;
        };

        return DeclTokenVisitor{compiler, position, callback}.Execute();
    }
} // namespace glsld