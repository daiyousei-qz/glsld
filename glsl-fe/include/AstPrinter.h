#pragma once
#include "Ast.h"
#include "AstVisitor.h"

#include <fmt/format.h>

namespace glsld
{
    class AstPrinter : public AstVisitor<AstPrinter>
    {
    public:
        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            PrintIdent(true);
            depth += 1;
            Print("@{}[{}]", AstNodeTagToString(node.GetTag()), static_cast<const void*>(&node));
            return AstVisitPolicy::Traverse;
        }
        auto VisitAstNodeBase(AstNodeBase& node) -> void
        {
            // Print("\n");
        }

        auto ExitAstNodeBase(AstNodeBase& node) -> void
        {
            depth -= 1;
        }

        auto VisitAstQualType(AstQualType& type) -> void
        {
            PrintIdent();
            Print("TypeDesc={};", GetDebugName(type.GetTypeDesc()));
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            PrintIdent();
            if (decl.GetName().klass != TokenKlass::Error) {
                Print("Name={};", decl.GetName().text.Get());
            }
            else {
                Print("Name=<Error>;");
            }
        }
        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            PrintIdent();
            for (const auto& declarator : decl.GetDeclarators()) {
                if (declarator.declTok.klass != TokenKlass::Error) {
                    Print("Name={};", declarator.declTok.text.Get());
                }
                else {
                    Print("Name=<Error>;");
                }
            }
        }

        auto VisitAstConstantExpr(AstConstantExpr& expr)
        {
            PrintIdent();
            Print("Value={};", expr.GetToken().text.Get());
            PrintAstExprPayload(expr);
        }

        auto VisitAstUnaryExpr(AstUnaryExpr& expr)
        {
            PrintIdent();
            Print("Op={};", UnaryOpToString(expr.GetOperator()));
            PrintAstExprPayload(expr);
        }

        auto VisitAstBinaryExpr(AstBinaryExpr& expr)
        {
            PrintIdent();
            Print("Op={};", BinaryOpToString(expr.GetOperator()));
            PrintAstExprPayload(expr);
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr)
        {
            PrintIdent();
            PrintAstExprPayload(expr);
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            PrintIdent();
            if (expr.GetAccessName().klass != TokenKlass::Error) {
                Print("Name={}; ", expr.GetAccessName().text.Get());
            }
            else {
                Print("Name=<Error>;");
            }

            Print("AccessType={}; AccessedDecl={}; ", NameAccessTypeToString(expr.GetAccessType()),
                  static_cast<const void*>(expr.GetAccessedDecl()));
            PrintAstExprPayload(expr);
        }

        auto Export() -> std::string
        {
            return std::move(buffer);
        }

    private:
        auto GetDebugName(const TypeDesc* typeDesc) -> std::string
        {
            if (typeDesc && !typeDesc->GetDebugName().empty()) {
                return std::string{typeDesc->GetDebugName()};
            }
            else {
                return fmt::format("{}", static_cast<const void*>(typeDesc));
            }
        }

        auto PrintAstExprPayload(AstExpr& expr) -> void
        {
            Print("DeducedType={}; ContextualType={}; ", GetDebugName(expr.GetDeducedType()),
                  GetDebugName(expr.GetContextualType()));
        }

        template <typename... Args>
        auto Print(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
        }

        auto PrintIdent(bool isNode = false) -> void
        {
            if (!buffer.empty()) {
                Print("\n");
            }

            for (int i = 0; i < depth; ++i) {

                if (isNode && i + 1 == depth) {
                    Print("'--");
                }
                else {
                    Print("|  ");
                }
            }
        }

        std::string buffer;
        int depth = 0;
    };
} // namespace glsld