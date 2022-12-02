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
            astContext = &ast;
            TypeCheckVisitor{*this}.TraverseAst(ast);
            astContext = nullptr;
        }

    private:
        // Type

        auto ResolveType(AstQualType& type) -> void
        {
            // FIXME: set correct type
            if (auto builtinType = GetBuiltinType(type.GetTypeNameTok())) {
                type.SetTypeDesc(GetBuiltinTypeDesc(*builtinType));
            }
            else {
                type.SetTypeDesc(GetErrorTypeDesc());
            }
        }

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
            decl.SetTypeDesc(astContext->CreateStructType(&decl));
        }
        auto DeclareVariable(AstVariableDecl& decl) -> void
        {
            auto type = decl.GetType()->GetTypeDesc();
            for (const auto& declarator : decl.GetDeclarators()) {
                TryAddSymbol(declarator.declTok.text, decl);
            }
        }
        auto DeclareInterfaceBlock(AstInterfaceBlockDecl& decl) -> void
        {
            if (!decl.GetDeclarator()) {
                // For unnamed interface block, names of internal members should be added to the current scope
            }
            decl.SetTypeDesc(astContext->CreateStructType(&decl));
        }
        auto DeclareParameter(AstParamDecl& decl) -> void
        {
            if (decl.GetDeclTok()) {
                TryAddSymbol(decl.GetDeclTok()->text, decl);
            }
        }

        // NOTE this is before children nodes are visited
        auto EnterFunctionScope(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(!InFunctionScope());

            currentFunction = &decl;
            symbolTable.PushScope();
        }
        auto ExitFunctionScope(AstFunctionDecl& decl) -> void
        {
            GLSLD_ASSERT(InFunctionScope());

            currentFunction = nullptr;
            symbolTable.PopScope();

            if (decl.GetName().klass != TokenKlass::Error) {
                symbolTable.AddFunction(decl);
            }
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
                if (expr.GetAccessType() == NameAccessType::Variable) {
                    auto accessName = expr.GetAccessName().text.Str();
                    auto symbol     = symbolTable.FindSymbol(accessName);
                    if (symbol) {
                        if (auto varDecl = symbol->As<AstVariableDecl>()) {
                            expr.SetAccessedDecl(symbol);
                            // FIXME: handle array type
                            expr.SetDeducedType(varDecl->GetType()->GetTypeDesc());
                        }
                        else if (auto paramDecl = symbol->As<AstParamDecl>()) {
                            expr.SetAccessedDecl(symbol);
                            // FIXME: handle array type
                            expr.SetDeducedType(varDecl->GetType()->GetTypeDesc());
                        }
                    }
                }

                // NOTE we resolve function call in CheckInvokeExpr
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
            expr.SetDeducedType(GetErrorTypeDesc());

            // FIXME: handle function call
            // FIXME: handle things like `S[2](...)`
            if (auto invokedExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {

                // resolve first
                auto accessName = invokedExpr->GetAccessName().text.Str();
                std::vector<const TypeDesc*> argTypes;
                for (auto argExpr : expr.GetArguments()) {
                    argTypes.push_back(argExpr->GetDeducedType());
                }

                switch (invokedExpr->GetAccessType()) {
                case NameAccessType::Constructor:
                {
                    if (auto builtinType = GetBuiltinType(invokedExpr->GetAccessName())) {
                        // Builtin types
                    }
                    else {
                        // User-defined types
                        auto symbol = symbolTable.FindSymbol(accessName);
                        if (symbol && (symbol->Is<AstStructDecl>() || symbol->Is<AstInterfaceBlockDecl>())) {
                            invokedExpr->SetAccessedDecl(symbol);
                        }
                    }

                    break;
                }
                case NameAccessType::Function:
                {
                    auto funcSymbol = symbolTable.FindFunction(accessName, argTypes);
                    if (funcSymbol) {
                        invokedExpr->SetAccessedDecl(funcSymbol);
                        invokedExpr->SetDeducedType(funcSymbol->GetReturnType()->GetTypeDesc());
                    }
                    break;
                }
                default:
                    GLSLD_UNREACHABLE();
                }

                if (invokedExpr->GetAccessName().klass != TokenKlass::Identifier) {
                    // This is a constructor
                    auto builtinType = GetBuiltinType(invokedExpr->GetAccessName());
                    GLSLD_ASSERT(builtinType.has_value());
                    expr.SetDeducedType(GetBuiltinTypeDesc(*builtinType));
                    return;
                }
            }
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

            auto ExitAstQualType(AstQualType& type) -> void
            {
                typeChecker.ResolveType(type);
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
                typeChecker.ExitFunctionScope(decl);
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
            auto ExitAstParamDecl(AstParamDecl& decl) -> void
            {
                typeChecker.DeclareParameter(decl);
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
        AstContext* astContext           = nullptr;
        SymbolTable symbolTable;
    };
} // namespace glsld