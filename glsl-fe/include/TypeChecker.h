#pragma once
#include "AstVisitor.h"
#include "SymbolTable.h"
#include "Typing.h"

#include <optional>

namespace glsld
{
    class TypeChecker
    {
    public:
        auto TypeCheck(AstContext& ast)
        {
            TypeCheckVisitor{*this}.TraverseAst(ast);
        }

    private:
        //
        // Global Decl
        //

        auto InFunctionScope() -> bool
        {
            return currentFunction != nullptr;
        }

        auto DeclareStructType(AstStructDecl& decl) -> void
        {
            if (decl.GetDeclToken()) {
                TryAddSymbol(decl.GetDeclToken()->text, decl);
            }
        }
        auto DeclareVariable(AstVariableDecl& decl) -> void
        {
            for (const auto& declarator : decl.GetDeclarators()) {
                TryAddSymbol(declarator.declTok.text, decl);
            }
        }
        auto DeclareInterfaceBlock(AstInterfaceBlockDecl& decl) -> void
        {
        }

        auto EnterFunctionScope(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(!InFunctionScope());
            currentFunction = &decl;
            symbolTable.PushScope();

            if (decl.GetName().klass != TokenKlass::Error) {
                auto funcName = decl.GetName().text.StrView();
                if (!funcName.empty()) {
                    symbolTable.AddSymbol(std::string{funcName}, &decl);
                }
            }
        }
        auto ExitFunctionScope() -> void
        {
            currentFunction = nullptr;
            symbolTable.PopScope();
        }

        auto EnterBlockScope() -> void
        {
            GLSLD_ASSERT(InFunctionScope());
            symbolTable.PushScope();
        }
        auto ExitBlockScope() -> void
        {
            GLSLD_ASSERT(InFunctionScope());
            symbolTable.PopScope();
        }

        //
        // Expr
        //
        // PreprocessXXXExpr: action before children are type checked
        // CheckXXXExpr: action after children are type checked

        auto PreprocessInvokeExpr(AstInvokeExpr& expr) -> void
        {
            // FIXME: handle construct like `float[4](...)`
            if (auto func = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (func->GetAccessName().klass != TokenKlass::Identifier) {
                    // This is a constructor
                    func->SetAccessType(NameAccessType::Constructor);
                }
                else {
                    func->SetAccessType(NameAccessType::Function);
                }
            }
        }

        auto CheckErrorExpr(AstErrorExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckConstantExpr(AstConstantExpr& expr) -> void
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
        auto CheckNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());

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
        auto CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckUnaryExpr(AstUnaryExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckBinaryExpr(AstBinaryExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckSelectExpr(AstSelectExpr& expr) -> void
        {
            expr.SetDeducedType(GetErrorTypeDesc());
        }
        auto CheckInvokeExpr(AstInvokeExpr& expr) -> void
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

        //
        // Type Eval
        //
        auto HasConversion(const TypeDesc* from, const TypeDesc* to) -> bool
        {
            GLSLD_NO_IMPL();
        }

        auto EvalUnary(UnaryOp op, const TypeDesc* oparand) -> const TypeDesc*
        {
            GLSLD_NO_IMPL();
        }

        auto EvalBinary(BinaryOp op, const TypeDesc* lhs, const TypeDesc* rhs) -> const TypeDesc*
        {
            GLSLD_NO_IMPL();
        }

    private:
        // FIXME: avoid doing another traversal. We could do this during parsing
        class TypeCheckVisitor : public AstVisitor<TypeCheckVisitor>
        {
        public:
            TypeCheckVisitor(TypeChecker& typeChecker) : typeChecker(typeChecker)
            {
            }

            auto EnterAstFunctionDecl(AstFunctionDecl& decl) -> AstVisitPolicy
            {
                if (typeChecker.InFunctionScope()) {
                    // nested function isn't allowed
                    return AstVisitPolicy::Leave;
                }

                return AstVisitPolicy::Traverse;
            }

            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                typeChecker.EnterFunctionScope(decl);
            }
            auto ExitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                typeChecker.ExitFunctionScope();
            }

            auto ExitAstStructDecl(AstStructDecl& decl) -> void
            {
                typeChecker.DeclareStructType(decl);
            }
            auto ExitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                typeChecker.DeclareVariable(decl);
            }
            auto ExitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
            {
                typeChecker.DeclareInterfaceBlock(decl);
            }

            auto VisitAstCompoundStmt(AstCompoundStmt& stmt) -> void
            {
                // ignore the outermost compound stmt because we share scope with the function decl
                if (compoundStmtDepth > 0) {
                    typeChecker.EnterBlockScope();
                }
                compoundStmtDepth += 1;
            }
            auto ExitAstCompoundStmt(AstCompoundStmt& stmt) -> void
            {
                // ignore the outermost compound stmt because we share scope with the function decl
                compoundStmtDepth -= 1;
                if (compoundStmtDepth > 0) {
                    typeChecker.ExitBlockScope();
                }
            }

            auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
            {
                typeChecker.PreprocessInvokeExpr(expr);
            }

            auto ExitAstExpr(AstExpr& expr) -> void
            {
                switch (expr.GetTag()) {
                case AstNodeTag::AstErrorExpr:
                    typeChecker.CheckErrorExpr(*expr.As<AstErrorExpr>());
                    break;
                case AstNodeTag::AstConstantExpr:
                    typeChecker.CheckConstantExpr(*expr.As<AstConstantExpr>());
                    break;
                case AstNodeTag::AstNameAccessExpr:
                    typeChecker.CheckNameAccessExpr(*expr.As<AstNameAccessExpr>());
                    break;
                case AstNodeTag::AstUnaryExpr:
                    typeChecker.CheckUnaryExpr(*expr.As<AstUnaryExpr>());
                    break;
                case AstNodeTag::AstBinaryExpr:
                    typeChecker.CheckBinaryExpr(*expr.As<AstBinaryExpr>());
                    break;
                case AstNodeTag::AstSelectExpr:
                    typeChecker.CheckSelectExpr(*expr.As<AstSelectExpr>());
                    break;
                case AstNodeTag::AstInvokeExpr:
                    typeChecker.CheckInvokeExpr(*expr.As<AstInvokeExpr>());
                    break;
                case AstNodeTag::AstIndexAccessExpr:
                    typeChecker.CheckIndexAccessExpr(*expr.As<AstIndexAccessExpr>());
                    break;
                default:
                    GLSLD_UNREACHABLE();
                }
            }

        private:
            TypeChecker& typeChecker;

            int compoundStmtDepth = 0;
        };

        auto TryAddSymbol(LexString name, AstDecl& decl) -> void
        {
            auto s = name.StrView();
            if (!s.empty()) {
                symbolTable.AddSymbol(std::string{s}, &decl);
            }
        }

        AstFunctionDecl* currentFunction = nullptr;
        SymbolTable symbolTable;
    };
} // namespace glsld