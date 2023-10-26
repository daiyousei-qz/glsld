#pragma once
#include "Compiler.h"
#include "AstContext.h"
#include "TypeContext.h"
#include "AstVisitor.h"
#include "SymbolTable.h"
#include "Typing.h"

#include <charconv>
#include <optional>

namespace glsld
{
    // The `TypeChecker` go through the entire Ast and resolve semantic information, including:
    // 1. Type check the program
    // 2. Compute the def-use relations
    // 3. Evaluate constant expressions
    // The computed information is filled into the respective payload that's attached to each Ast node
    class TypeChecker
    {
    public:
        TypeChecker(CompilerObject& compilerObject) : compilerObject(compilerObject)
        {
        }

        auto DoTypeCheck(const SymbolTable* externalSymbolTable) -> std::unique_ptr<SymbolTable>;

    private:
        //
        // Type
        //

        auto ResolveType(AstQualType& type) -> void;

        //
        // Global Decl
        //

        auto InFunctionScope() const noexcept -> bool
        {
            return currentFunction != nullptr;
        }

        auto DeclareStruct(AstStructDecl& decl) -> void;

        auto DeclareInterfaceBlock(AstInterfaceBlockDecl& decl) -> void;

        auto DeclareVariable(AstVariableDecl& decl) -> void;

        auto DeclareParameter(AstParamDecl& decl) -> void;

        // NOTE this is before children nodes are visited
        auto EnterFunctionScope(AstFunctionDecl& decl) -> void;

        auto ExitFunctionScope(AstFunctionDecl& decl) -> void;

        auto EnterBlockScope() -> void;

        auto ExitBlockScope() -> void;

        //
        // Expr
        //
        // PreprocessXXXExpr: action before children are type checked
        // CheckXXXExpr: action after children are type checked

        // For `InvokeExpr`, we need to update the access type before entering the children nodes
        // so we could skip the type checking of the invoked expression.
        // We'll do the checking in `CheckInvokeExpr` because of better context.
        auto PreprocessInvokeExpr(AstFunctionCallExpr& expr) -> void;

        auto CheckConstantExpr(AstLiteralExpr& expr) -> void;

        auto CheckNameAccessExpr(AstNameAccessExpr& expr) -> void;

        auto CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void;

        auto CheckUnaryExpr(AstUnaryExpr& expr) -> void;

        auto CheckBinaryExpr(AstBinaryExpr& expr) -> void;

        auto CheckSelectExpr(AstSelectExpr& expr) -> void;

        auto CheckInvokeExpr(AstFunctionCallExpr& expr) -> void;

        //
        // Stmt
        //

        auto CheckJumpStmt(AstJumpStmt& stmt) -> void
        {
        }
        auto CheckReturnStmt(AstReturnStmt& stmt) -> void
        {
        }

        //
        // Type Eval
        //
        auto GetDeclType(DeclView declView) -> const Type*;

    private:
        friend class TypeCheckVisitor;

        auto ResolveSwizzleAccess(AstNameAccessExpr& expr) -> void;

        // Resolve the called expression to a function or a constructor.
        //
        // The following type of expression could be invoked:
        // 1. `func()` where `func` is a function name
        // 2. `type()` where `type` is a type specifier
        // 3. `expr.length()` where `expr` has array/vector type
        auto ResolveInvokeExpr(AstFunctionCallExpr& expr) -> void;

        auto GetSymbolTable() -> SymbolTable&
        {
            GLSLD_ASSERT(!symbolTableStack.empty());
            return *symbolTableStack.back();
        }

        auto PushScope() -> void
        {
            symbolTableStack.push_back(std::make_unique<SymbolTable>(&GetSymbolTable()));
        }

        auto PopScope() -> void
        {
            symbolTableStack.pop_back();
        }

        auto FindFunction(const std::string& name, const std::vector<const Type*>& argTypes) -> AstFunctionDecl*
        {
            // FIXME: what about overload resolution between external and internal functions?

            // NOTE we directly search in the global scope symbol table since functions cannot be declared locally
            if (auto funcDecl = symbolTableStack[0]->FindFunction(name, argTypes)) {
                return funcDecl;
            }
            else {
                return externalSymbolTable ? externalSymbolTable->FindFunction(name, argTypes) : nullptr;
            }
        }

        auto FindSymbol(const std::string& name) -> DeclView
        {
            return GetSymbolTable().FindSymbol(name);
        }

        CompilerObject& compilerObject;

        const SymbolTable* externalSymbolTable = nullptr;

        AstFunctionDecl* currentFunction = nullptr;
        std::vector<std::unique_ptr<SymbolTable>> symbolTableStack;
    };
} // namespace glsld