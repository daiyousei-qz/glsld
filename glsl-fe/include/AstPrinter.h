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
            Print("{}@{} ", AstNodeTagToString(node.GetTag()), static_cast<void*>(&node));
            return AstVisitPolicy::Traverse;
        }
        auto ExitAstNodeBase(AstNodeBase& node) -> void
        {
            depth -= 1;
        }

        auto VisitAstNodeBase(AstNodeBase& node) -> void
        {
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
            Print("[Value={};]\n", expr.GetToken().text.Get());
        }

        auto VisitAstUnaryExpr(AstUnaryExpr& expr)
        {
            Print("[Op={};]\n", UnaryOpToString(expr.GetOperator()));
        }

        auto VisitAstBinaryExpr(AstBinaryExpr& expr)
        {
            Print("[Op={};]\n", BinaryOpToString(expr.GetOperator()));
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr)
        {
            Print("[Type={};]\n", InvocationTypeToString(expr.GetInvocationType()));
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
            Print("\n");
        }

        auto Export() -> std::string
        {
            return std::move(buffer);
        }

    private:
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