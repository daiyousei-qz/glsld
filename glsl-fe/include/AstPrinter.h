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
            PrintIdent();
            depth += 1;
            Print("{}@{} ", AstNodeTagToString(node.GetTag()), static_cast<const void*>(&node));
            return AstVisitPolicy::Traverse;
        }
        auto VisitAstNodeBase(AstNodeBase& node) -> void
        {
            Print("\n");
        }

        auto ExitAstNodeBase(AstNodeBase& node) -> void
        {
            depth -= 1;
        }

        auto VisitAstQualType(AstQualType& type) -> void
        {
            Print("[TypeDesc={};]", GetDebugName(type.GetTypeDesc()));
            Print("\n");
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            if (decl.GetName().klass != TokenKlass::Error) {
                Print("[Name={};]\n", decl.GetName().text.Get());
            }
            else {
                Print("[Name=<Error>;]\n");
            }
        }
        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            Print("[");
            for (const auto& declarator : decl.GetDeclarators()) {
                if (declarator.declTok.klass != TokenKlass::Error) {
                    Print("Name={};", declarator.declTok.text.Get());
                }
                else {
                    Print("Name=<Error>;");
                }
            }
            Print("]\n");
        }

        auto VisitAstConstantExpr(AstConstantExpr& expr)
        {
            Print("[Value={};]", expr.GetToken().text.Get());
            PrintAstExprPayload(expr);
            Print("\n");
        }

        auto VisitAstUnaryExpr(AstUnaryExpr& expr)
        {
            Print("[Op={};]", UnaryOpToString(expr.GetOperator()));
            PrintAstExprPayload(expr);
            Print("\n");
        }

        auto VisitAstBinaryExpr(AstBinaryExpr& expr)
        {
            Print("[Op={};]", BinaryOpToString(expr.GetOperator()));
            PrintAstExprPayload(expr);
            Print("\n");
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr)
        {
            Print("[]");
            PrintAstExprPayload(expr);
            Print("\n");
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            if (expr.GetAccessName().klass != TokenKlass::Error) {
                Print("[Name={};] ", expr.GetAccessName().text.Get());
            }
            else {
                Print("[Name=<Error>;]");
            }

            Print("[AccessType={}; AccessedDecl={};] ", NameAccessTypeToString(expr.GetAccessType()),
                  static_cast<const void*>(expr.GetAccessedDecl()));
            PrintAstExprPayload(expr);
            Print("\n");
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
            Print("[DeducedType={}; ContextualType={};] ", GetDebugName(expr.GetDeducedType()),
                  GetDebugName(expr.GetContextualType()));
        }

        template <typename... Args>
        auto Print(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
        }

        auto PrintIdent() -> void
        {
            for (int i = 0; i < depth; ++i) {
                Print("  ");
            }
        }

        std::string buffer;
        int depth = 0;
    };
} // namespace glsld