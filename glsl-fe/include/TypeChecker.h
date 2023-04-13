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

        auto PreprocessInvokeExpr(AstInvokeExpr& expr) -> void;

        auto CheckErrorExpr(AstErrorExpr& expr) -> void;

        auto CheckConstantExpr(AstConstantExpr& expr) -> void;

        auto CheckNameAccessExpr(AstNameAccessExpr& expr) -> void;

        auto CheckIndexAccessExpr(AstIndexAccessExpr& expr) -> void;

        auto CheckUnaryExpr(AstUnaryExpr& expr) -> void;

        auto CheckBinaryExpr(AstBinaryExpr& expr) -> void;

        auto CheckSelectExpr(AstSelectExpr& expr) -> void;

        auto CheckInvokeExpr(AstInvokeExpr& expr) -> void;

        //
        // Type Eval
        //
        auto GetDeclType(DeclView declView) -> const Type*;

    private:
        friend class TypeCheckVisitor;

        auto ResolveSwizzleAccess(AstNameAccessExpr& expr) -> void
        {
            GLSLD_ASSERT(expr.GetAccessChain()->GetDeducedType()->IsScalar() ||
                         expr.GetAccessChain()->GetDeducedType()->IsVector());
            auto swizzleName = expr.GetAccessName().text.StrView();

            if (swizzleName.Size() <= 4) {
                // FIXME: set correct scalar type
                expr.SetAccessType(NameAccessType::Swizzle);
                expr.SetDeducedType(GetVectorTypeDesc(ScalarType::Float, swizzleName.Size()));
            }
        }

        // Resolve identifier
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
                std::vector<const Type*> argTypes;
                for (auto argExpr : expr.GetArguments()) {
                    argTypes.push_back(argExpr->GetDeducedType());
                }

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
                    auto funcSymbol = FindFunction(accessName, argTypes);
                    if (funcSymbol) {
                        // FIXME: invoked expr should have deduced type of function?
                        invokedExpr->SetAccessedDecl(DeclView{funcSymbol});
                        expr.SetDeducedType(funcSymbol->GetReturnType()->GetResolvedType());
                    }
                    break;
                }
                default:
                    GLSLD_UNREACHABLE();
                }
            }
            else if (auto invokedExpr = expr.GetInvokedExpr()->As<AstIndexAccessExpr>();
                     invokedExpr && invokedExpr->GetInvokedExpr()->Is<AstNameAccessExpr>()) {
                // Case 2: `Type[...]()`
                invokedExpr->GetInvokedExpr()->As<AstNameAccessExpr>()->SetAccessType(NameAccessType::Constructor);
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