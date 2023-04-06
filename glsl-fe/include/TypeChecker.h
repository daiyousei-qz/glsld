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

        auto TypeCheck(const SymbolTable* externalSymbolTable) -> SymbolTable;

    private:
        //
        // Type
        //

        auto ResolveType(AstQualType& type) -> void
        {
            const Type* resolvedType = GetErrorTypeDesc();

            // Resolve element type
            if (auto builtinType = GetGlslBuiltinType(type.GetTypeNameTok())) {
                resolvedType = GetBuiltinTypeDesc(*builtinType);
            }
            else if (type.GetStructDecl()) {
                // A struct type
                // NOTE it is already resolved
                resolvedType = type.GetStructDecl()->GetTypeDesc();
            }
            else {
                auto symbol = FindSymbol(type.GetTypeNameTok().text.Str());
                if (symbol && symbol.GetDecl()->Is<AstStructDecl>()) {
                    resolvedType = symbol.GetDecl()->As<AstStructDecl>()->GetTypeDesc();
                    type.SetResolvedStructDecl(symbol.GetDecl()->As<AstStructDecl>());
                }
            }

            // Resolve array type if any
            resolvedType = compilerObject.GetTypeContext().GetArrayType(resolvedType, type.GetArraySpec());

            type.SetResolvedType(resolvedType);
        }

        //
        // Global Decl
        //

        auto InFunctionScope() -> bool;

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
        auto GetDeclType(DeclView declView) -> const Type*
        {
            if (!declView.IsValid()) {
                return GetErrorTypeDesc();
            }

            auto decl = declView.GetDecl();
            if (decl->Is<AstEmptyDecl>()) {
                return GetErrorTypeDesc();
            }
            else if (auto structDecl = decl->As<AstStructDecl>()) {
                return structDecl->GetTypeDesc();
            }
            else if (auto blockDecl = decl->As<AstInterfaceBlockDecl>()) {
                return blockDecl->GetTypeDesc();
            }
            else if (auto memberDecl = decl->As<AstStructMemberDecl>()) {
                if (declView.GetIndex() < memberDecl->GetDeclarators().size()) {
                    return compilerObject.GetTypeContext().GetArrayType(
                        memberDecl->GetType()->GetResolvedType(),
                        memberDecl->GetDeclarators()[declView.GetIndex()].arraySize);
                }
            }
            else if (auto varDecl = decl->As<AstVariableDecl>()) {
                if (declView.GetIndex() < varDecl->GetDeclarators().size()) {
                    return compilerObject.GetTypeContext().GetArrayType(
                        varDecl->GetType()->GetResolvedType(),
                        varDecl->GetDeclarators()[declView.GetIndex()].arraySize);
                }
            }
            else if (auto paramDecl = decl->As<AstParamDecl>()) {
                // FIXME: arrayness
                return paramDecl->GetType()->GetResolvedType();
            }
            else if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                // FIXME: function type?
                return GetErrorTypeDesc();
            }

            GLSLD_UNREACHABLE();
        }

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
                        auto symbol = symbolTable.FindSymbol(accessName);
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

        auto FindFunction(const std::string& name, const std::vector<const Type*>& argTypes) -> AstFunctionDecl*
        {
            if (auto funcDecl = symbolTable.FindFunction(name, argTypes)) {
                return funcDecl;
            }
            else {
                return externalSymbolTable ? externalSymbolTable->FindFunction(name, argTypes) : nullptr;
            }
        }

        auto FindSymbol(const std::string& name) -> DeclView
        {
            return symbolTable.FindSymbol(name);
        }

        CompilerObject& compilerObject;

        const SymbolTable* externalSymbolTable = nullptr;

        AstFunctionDecl* currentFunction = nullptr;
        SymbolTable symbolTable;
    };
} // namespace glsld