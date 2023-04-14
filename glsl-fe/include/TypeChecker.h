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
        auto PreprocessInvokeExpr(AstInvokeExpr& expr) -> void;

        auto CheckConstantExpr(AstConstantExpr& expr) -> void;

        auto CheckNameAccessExpr(AstNameAccessExpr& expr) -> void;

        auto CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void;

        auto CheckUnaryExpr(AstUnaryExpr& expr) -> void;

        auto CheckBinaryExpr(AstBinaryExpr& expr) -> void;

        auto CheckSelectExpr(AstSelectExpr& expr) -> void;

        auto CheckInvokeExpr(AstInvokeExpr& expr) -> void;

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

        auto ResolveSwizzleAccess(AstNameAccessExpr& expr) -> void
        {
            auto swizzleName = expr.GetAccessName().text.StrView();
            if (swizzleName.Size() < 1 || swizzleName.Size() > 4) {
                // FIXME: report error, bad swizzle
                return;
            }

            expr.SetAccessType(NameAccessType::Swizzle);

            const auto baseExprType = expr.GetAccessChain()->GetDeducedType();
            int baseExprDimSize     = 0;
            if (baseExprType->IsScalar()) {
                baseExprDimSize = 1;
                expr.SetDeducedType(GetVectorTypeDesc(baseExprType->GetScalarDesc()->type, swizzleName.Size()));
            }
            else if (baseExprType->IsVector()) {
                baseExprDimSize = baseExprType->GetVectorDesc()->vectorSize;
                expr.SetDeducedType(GetVectorTypeDesc(baseExprType->GetVectorDesc()->scalarType, swizzleName.Size()));
            }
            else {
                GLSLD_ASSERT(false);
            }

            // -> {swizzleSet, swizzleIndex}
            auto translateSwizzleChar = [](char ch) -> std::pair<int, int> {
                switch (ch) {
                case 'x':
                    return {0, 0};
                case 'y':
                    return {0, 1};
                case 'z':
                    return {0, 2};
                case 'w':
                    return {0, 3};
                case 'r':
                    return {1, 0};
                case 'g':
                    return {1, 1};
                case 'b':
                    return {1, 2};
                case 'a':
                    return {1, 3};
                case 's':
                    return {2, 0};
                case 't':
                    return {2, 1};
                case 'p':
                    return {2, 2};
                case 'q':
                    return {2, 3};
                default:
                    return {-1, 0};
                }
            };

            int lastSwizzleSet = -1;
            uint8_t swizzleBuffer[4];

            for (int i = 0; i < swizzleName.Size(); ++i) {
                const auto [set, index] = translateSwizzleChar(swizzleName[i]);
                if (set == -1) {
                    // FIXME: report error, bad swizzle char
                }
                else if (lastSwizzleSet != -1 && set != lastSwizzleSet) {
                    // FIXME: report error, swizzle set mismatch
                }
                else if (index >= baseExprDimSize) {
                    // FIXME: report error, swizzle index out of range
                }

                lastSwizzleSet   = set;
                swizzleBuffer[i] = static_cast<uint8_t>(index);
            }

            expr.SetSwizzleInfo(SwizzleDesc{ArrayView<uint8_t>{swizzleBuffer, swizzleName.Size()}});
        }

        // Resolve the called expression to a function or a constructor.
        //
        // The following type of expression could be invoked:
        // 1. `func()` where `func` is a function name
        // 2. `type()` where `type` is a type specifier
        // 3. `expr.length()` where `expr` has array/vector type
        auto ResolveInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto invokedExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                // Case 1: `ID()`
                auto accessName = invokedExpr->GetAccessName().text.Str();

                switch (invokedExpr->GetAccessType()) {
                case NameAccessType::Constructor:
                {
                    if (auto builtinType = GetGlslBuiltinType(invokedExpr->GetAccessName())) {
                        // Builtin types
                    }
                    else {
                        // User-defined types
                        auto symbol = FindSymbol(accessName);
                        if (symbol &&
                            (symbol.GetDecl()->Is<AstStructDecl>() || symbol.GetDecl()->Is<AstInterfaceBlockDecl>())) {
                            invokedExpr->SetAccessedDecl(symbol);
                        }
                    }

                    break;
                }
                case NameAccessType::Function:
                {
                    // FIXME: Check if the identifier is shadowed by a local variable
                    // FIXME: We might need to collect all functions from the symbol table and then resolute the
                    // overload
                    std::vector<const Type*> argTypes;
                    for (auto argExpr : expr.GetArguments()) {
                        argTypes.push_back(argExpr->GetDeducedType());
                    }
                    auto funcSymbol = FindFunction(accessName, argTypes);
                    if (funcSymbol) {
                        // FIXME: invoked expr should have deduced type of function?
                        invokedExpr->SetAccessedDecl(DeclView{funcSymbol});
                        expr.SetDeducedType(funcSymbol->GetResolvedReturnType());
                    }
                    break;
                }
                default:
                    GLSLD_UNREACHABLE();
                }
            }

            // Case 3: `expr.length()`
        }

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