#pragma once
#include "AstVisitor.h"
#include "SymbolTable.h"
#include "Typing.h"

#include <optional>

namespace glsld
{
    // FIXME: avoid doing another traversal. We could do this during parsing
    class TypeChecker : public AstVisitor<TypeChecker>
    {
    public:
        auto EnterAstFunctionDecl(AstFunctionDecl& decl) -> AstVisitPolicy
        {
            if (currentFunc) {
                // nested function isn't allowed
                return AstVisitPolicy::Leave;
            }

            return AstVisitPolicy::Traverse;
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            if (decl.GetName().klass != TokenKlass::Error) {
                currentFunc = decl.GetName().text.Str();
                symbolTable.AddSymbol(*currentFunc, &decl);
            }
            else {
                currentFunc = "";
            }
            symbolTable.PushScope();
        }
        auto ExitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            currentFunc = std::nullopt;
            symbolTable.PopScope();
        }

        auto VisitAstStructDecl(AstStructDecl& decl) -> void
        {
            if (decl.GetDeclToken()) {
                TryAddSymbol(decl.GetDeclToken()->text, decl);
            }
        }
        auto VisitAstParamDecl(AstParamDecl& decl) -> void
        {
            TryAddSymbol(decl.GetDeclTok().text, decl);
        }
        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            for (const auto& declarator : decl.GetDeclarators()) {
                TryAddSymbol(declarator.declTok.text, decl);
            }
        }

        auto VisitAstCompoundStmt(AstCompoundStmt& stmt) -> void
        {
            // ignore the outermost compound stmt because we share scope with the function decl
            if (compoundStmtDepth > 0) {
                symbolTable.PushScope();
            }
            compoundStmtDepth += 1;
        }
        auto ExitAstCompoundStmt(AstCompoundStmt& stmt) -> void
        {
            // ignore the outermost compound stmt because we share scope with the function decl
            compoundStmtDepth -= 1;
            if (compoundStmtDepth > 0) {
                symbolTable.PopScope();
            }
        }

        auto ExitAstConstantExpr(AstConstantExpr& expr) -> void
        {
            // FIXME: handle literals with typing suffix
            switch (expr.GetToken().klass) {
            case TokenKlass::K_true:
            case TokenKlass::K_false:
                expr.SetDeducedType(GetBuiltinTypeDesc(BuiltinType::Ty_bool));
                break;
            case TokenKlass::IntegerConstant:
                expr.SetDeducedType(GetBuiltinTypeDesc(BuiltinType::Ty_int));
                break;
            case TokenKlass::FloatConstant:
                expr.SetDeducedType(GetBuiltinTypeDesc(BuiltinType::Ty_double));
                break;
            default:
                GLSLD_UNREACHABLE();
            }
        }

        auto ExitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            if (expr.GetAccessType() == NameAccessType::Unknown) {
                expr.SetAccessType(NameAccessType::Variable);
            }

            if (expr.GetAccessChain()) {
                // Accessing a member name
            }
            else {
                // Accessing a non-member name
                auto accessName = expr.GetAccessName().text.StrView();
                auto symbol     = symbolTable.FindSymbol(std::string{accessName});
                if (symbol) {
                    switch (expr.GetAccessType()) {
                    case NameAccessType::Constructor:
                        if (symbol->Is<AstStructDecl>() || symbol->Is<AstInterfaceBlockDecl>()) {
                            expr.SetAccessedDecl(symbol);
                        }
                    case NameAccessType::Function:
                        if (symbol->Is<AstFunctionDecl>()) {
                            expr.SetAccessedDecl(symbol);
                        }
                    case NameAccessType::Variable:
                    case NameAccessType::Unknown:
                        if (symbol->Is<AstVariableDecl>() || symbol->Is<AstParamDecl>()) {
                            expr.SetAccessedDecl(symbol);
                        }
                    }
                }
            }
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto func = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (func->GetAccessName().klass != TokenKlass::Identifier) {
                    // This is a constructor
                    func->SetAccessType(NameAccessType::Constructor);
                    return;
                }
                else {
                    func->SetAccessType(NameAccessType::Function);
                }
            }
        }

        auto ExitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            // FIXME: handle function call
            // FIXME: handle things like `S[2](...)`
            if (auto func = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (func->GetAccessName().klass != TokenKlass::Identifier) {
                    // This is a constructor
                    auto builtinType = GetBuiltinType(func->GetAccessName());
                    GLSLD_ASSERT(builtinType.has_value());
                    expr.SetDeducedType(GetBuiltinTypeDesc(*builtinType));
                    return;
                }
            }

            expr.SetDeducedType(GetErrorTypeDesc());
        }

        auto ExitAstExpr(AstExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());
        }

    private:
        auto TryAddSymbol(LexString name, AstDecl& decl) -> void
        {
            auto s = name.StrView();
            if (!s.empty()) {
                symbolTable.AddSymbol(std::string{s}, &decl);
            }
        }

        int compoundStmtDepth = 0;
        SymbolTable symbolTable;

        std::optional<std::string> currentFunc;
    };
} // namespace glsld